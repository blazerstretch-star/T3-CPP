#!/usr/bin/env python3
"""
Judge Calibration Module for t3_cpp

Validates LLM judge scores against human annotations to ensure:
1. Judge-human correlation is acceptable (≥0.65)
2. Persona weights are properly calibrated
3. Judge biases are identified and documented

Usage:
    # Step 1: Sample steps for human annotation
    python calibrate_judge.py sample \
        --input labeled_steps_with_judge.jsonl \
        --output human_annotation_batch.jsonl \
        --n_samples 300

    # Step 2: Annotate (manual - use annotation_interface.html)
    # Creates: human_annotations.jsonl

    # Step 3: Compute calibration metrics
    python calibrate_judge.py analyze \
        --judge_labels labeled_steps_with_judge.jsonl \
        --human_labels human_annotations.jsonl \
        --output calibration_report.json
"""

import argparse
import copy
import json
import random
from collections import defaultdict
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import numpy as np
from scipy.stats import pearsonr, spearmanr, kendalltau


# ═══════════════════════════════════════════════════════════════════════════
# STRATIFIED SAMPLING FOR HUMAN ANNOTATION
# ═══════════════════════════════════════════════════════════════════════════

def stratified_sample_for_annotation(
    steps: List[Dict],
    n_samples: int = 300,
    strata_config: Dict[str, int] = None
) -> List[Dict]:
    """
    Sample steps for human annotation with stratification.
    
    Stratification ensures coverage of:
    - Trajectory types (A-E)
    - Context types (baseline, local_completion, local_infilling)
    - Repositories (octree, voronoi, prepair, real_cpp_repo)
    - Edge cases (Type E, forced finals, high disagreement)
    
    Args:
        steps: All labeled steps
        n_samples: Total samples to draw
        strata_config: Custom strata sizes (default: balanced)
    
    Returns:
        List of sampled steps with annotation metadata
    """
    if strata_config is None:
        # Default: balanced across trajectory types + edge cases
        strata_config = {
            "type_a": 50,  # High-confidence success
            "type_b": 50,  # Recovery success
            "type_c": 50,  # Tested failure
            "type_d": 30,  # Untested failure
            "type_e": 50,  # High-confidence failure (critical!)
            "disagreement": 40,  # MC vs judge disagree
            "forced_final": 20,  # Forced submissions
            "random": 10,  # Pure random
        }
    
    # Group steps by strata
    strata = defaultdict(list)
    
    for step in steps:
        traj_type = step.get("judge_trajectory_type", "unknown")
        mc_soft = step.get("mc_label_soft", 0.5)
        judge_score = step.get("judge_overall_score", 0.5)
        is_forced = step.get("is_forced_final", False)
        
        # Trajectory type strata
        if traj_type == "high_confidence_success":
            strata["type_a"].append(step)
        elif traj_type == "recovery_success":
            strata["type_b"].append(step)
        elif traj_type == "tested_failure":
            strata["type_c"].append(step)
        elif traj_type == "untested_failure":
            strata["type_d"].append(step)
        elif traj_type == "high_confidence_failure":
            strata["type_e"].append(step)
        
        # Edge case strata
        if abs(mc_soft - judge_score) > 0.3:
            strata["disagreement"].append(step)
        
        if is_forced:
            strata["forced_final"].append(step)
        
        strata["random"].append(step)
    
    # Sample from each stratum
    sampled = []
    for stratum_name, target_count in strata_config.items():
        available = strata.get(stratum_name, [])
        if not available:
            print(f"Warning: No steps in stratum '{stratum_name}'")
            continue
        
        sample_size = min(target_count, len(available))
        samples = random.sample(available, sample_size)
        
        # Add stratum metadata (copy to avoid mutating source)
        for step in samples:
            step_copy = copy.copy(step)
            step_copy["annotation_stratum"] = stratum_name
            sampled.append(step_copy)
    
    # Deduplicate (a step may appear in multiple strata)
    seen_ids = set()
    deduped = []
    for step in sampled:
        step_id = (step.get("trajectory_id"), step.get("step_position"))
        if step_id not in seen_ids:
            seen_ids.add(step_id)
            deduped.append(step)
    
    # Shuffle to avoid ordering bias
    random.shuffle(deduped)
    
    return deduped[:n_samples]


# ═══════════════════════════════════════════════════════════════════════════
# HUMAN ANNOTATION SCHEMA
# ═══════════════════════════════════════════════════════════════════════════

def create_annotation_template(step: Dict) -> Dict:
    """
    Create annotation template for a single step.
    
    Human annotators will provide:
    1. Overall step quality (0-1)
    2. Dimension-specific scores (navigation, correctness, debugging, etc.)
    3. Pairwise preferences (optional)
    4. Free-text justification
    """
    return {
        # Identity
        "trajectory_id": step.get("trajectory_id"),
        "task_id": step.get("task_id"),
        "step_position": step.get("step_position"),
        "annotation_stratum": step.get("annotation_stratum"),
        
        # Context for annotator
        "action_type": step.get("action_type"),
        "tool_name": step.get("tool_name"),
        "reasoning": step.get("reasoning_normalized", "")[:500],
        "code": step.get("code", "")[:500],
        "observation_summary": str(step.get("observation", {}))[:300],
        "error_type": step.get("error_type"),
        "outcome": step.get("outcome"),
        
        # Judge predictions (for comparison)
        "judge_overall_score": step.get("judge_overall_score"),
        "judge_axes_used": step.get("judge_axes_used", []),
        "mc_label_soft": step.get("mc_label_soft"),
        "mc_advantage": step.get("mc_advantage"),
        
        # Human annotations (to be filled)
        "human_overall_quality": None,  # 0-1 scale
        "human_correctness_quality": None,  # 0-1 (if applicable)
        "human_reasoning_quality": None,  # 0-1 (if applicable)
        "human_action_quality": None,  # 0-1 (if applicable)
        "human_efficiency_quality": None,  # 0-1 (if applicable)
        "human_justification": None,  # Free text
        "annotator_id": None,  # Annotator identifier
        "annotation_time_seconds": None,  # Time spent
    }


# ═══════════════════════════════════════════════════════════════════════════
# CALIBRATION METRICS
# ═══════════════════════════════════════════════════════════════════════════

def compute_correlation_metrics(
    judge_scores: List[float],
    human_scores: List[float],
    metric_name: str = "overall"
) -> Dict:
    """
    Compute correlation between judge and human scores.
    
    Returns:
        - Pearson r (linear correlation)
        - Spearman ρ (rank correlation)
        - Kendall τ (pairwise agreement)
        - MAE (mean absolute error)
        - Bias (systematic over/under-estimation)
    """
    judge_scores = np.array(judge_scores)
    human_scores = np.array(human_scores)
    
    # Correlations
    pearson_r, pearson_p = pearsonr(judge_scores, human_scores)
    spearman_rho, spearman_p = spearmanr(judge_scores, human_scores)
    kendall_tau, kendall_p = kendalltau(judge_scores, human_scores)
    
    # Error metrics
    mae = np.mean(np.abs(judge_scores - human_scores))
    rmse = np.sqrt(np.mean((judge_scores - human_scores) ** 2))
    bias = np.mean(judge_scores - human_scores)
    
    # Agreement bins (how often judge and human agree on quality tier)
    def bin_score(s):
        if s >= 0.7: return "high"
        elif s >= 0.4: return "medium"
        else: return "low"
    
    judge_bins = [bin_score(s) for s in judge_scores]
    human_bins = [bin_score(s) for s in human_scores]
    agreement = sum(j == h for j, h in zip(judge_bins, human_bins)) / len(judge_bins)
    
    return {
        "metric": metric_name,
        "n_samples": len(judge_scores),
        "pearson_r": round(pearson_r, 3),
        "pearson_p": round(pearson_p, 4),
        "spearman_rho": round(spearman_rho, 3),
        "spearman_p": round(spearman_p, 4),
        "kendall_tau": round(kendall_tau, 3),
        "kendall_p": round(kendall_p, 4),
        "mae": round(mae, 3),
        "rmse": round(rmse, 3),
        "bias": round(bias, 3),
        "bin_agreement": round(agreement, 3),
        "interpretation": interpret_correlation(spearman_rho, mae, bias)
    }


def interpret_correlation(spearman_rho: float, mae: float, bias: float) -> str:
    """Interpret calibration quality using Spearman rho as primary metric."""
    if spearman_rho >= 0.75 and mae < 0.15:
        return "Excellent: Judge is well-calibrated — safe for PRM training"
    elif spearman_rho >= 0.65 and mae < 0.20:
        return "Good: Acceptable calibration — use with MC blend (0.6 MC / 0.4 judge)"
    elif spearman_rho >= 0.50 and mae < 0.25:
        return "Moderate: Marginal calibration — use judge for filtering only, not labels"
    elif spearman_rho >= 0.35:
        return "Poor: Weak calibration — do NOT use as training label; use for disagreement detection only"
    else:
        return "Unacceptable: Judge is uncorrelated with humans — do NOT use in pipeline"


def analyze_axis_calibration(
    annotations: List[Dict]
) -> Dict[str, Dict]:
    """
    Analyze calibration for each evaluation axis separately.
    
    Returns per-axis metrics to identify which axes need adjustment.
    """
    axis_metrics = {}
    
    # Group by axis
    axis_data = defaultdict(lambda: {"judge": [], "human": []})
    
    for ann in annotations:
        axes_used = ann.get("judge_axes_used", [])
        judge_axis_scores = ann.get("judge_axis_scores") or {}
        
        # Correctness
        if "correctness" in axes_used and ann.get("human_correctness_quality") is not None:
            axis_data["correctness"]["judge"].append(
                judge_axis_scores.get("correctness", 0.5)
            )
            axis_data["correctness"]["human"].append(ann["human_correctness_quality"])
        
        # Reasoning Coherence
        if "reasoning_coherence" in axes_used and ann.get("human_reasoning_quality") is not None:
            axis_data["reasoning_coherence"]["judge"].append(
                judge_axis_scores.get("reasoning_coherence", 0.5)
            )
            axis_data["reasoning_coherence"]["human"].append(ann["human_reasoning_quality"])
        
        # Action Appropriateness
        if "action_appropriateness" in axes_used and ann.get("human_action_quality") is not None:
            axis_data["action_appropriateness"]["judge"].append(
                judge_axis_scores.get("action_appropriateness", 0.5)
            )
            axis_data["action_appropriateness"]["human"].append(ann["human_action_quality"])
        
        # Efficiency
        if "efficiency" in axes_used and ann.get("human_efficiency_quality") is not None:
            axis_data["efficiency"]["judge"].append(
                judge_axis_scores.get("efficiency", 0.5)
            )
            axis_data["efficiency"]["human"].append(ann["human_efficiency_quality"])
    
    # Compute metrics for each axis
    for axis, data in axis_data.items():
        if len(data["judge"]) >= 20:  # Minimum 20 for statistical reliability
            axis_metrics[axis] = compute_correlation_metrics(
                data["judge"], data["human"], metric_name=axis
            )
        elif len(data["judge"]) >= 10:
            print(f"Warning: Only {len(data['judge'])} samples for {axis} — correlation may be unreliable")
    
    return axis_metrics


def detect_systematic_biases(annotations: List[Dict]) -> Dict:
    """
    Detect systematic biases in judge scores.
    
    Checks for:
    1. Trajectory type bias (does judge over/under-score certain types?)
    2. Repository bias (does judge favor certain codebases?)
    3. Length bias (does judge penalize longer trajectories?)
    4. Disagreement bias (does judge systematically differ from MC in certain cases?)
    """
    biases = {}
    
    # Trajectory type bias
    type_bias = defaultdict(lambda: {"judge": [], "human": []})
    for ann in annotations:
        # Use judge_trajectory_type if available, fallback to annotation_stratum
        traj_type = ann.get("judge_trajectory_type") or ann.get("annotation_stratum", "unknown")
        if ann.get("human_overall_quality") is not None and ann.get("judge_overall_score") is not None:
            type_bias[traj_type]["judge"].append(ann["judge_overall_score"])
            type_bias[traj_type]["human"].append(ann["human_overall_quality"])
    
    biases["trajectory_type"] = {}
    for traj_type, data in type_bias.items():
        if len(data["judge"]) >= 5:
            judge_mean = np.mean(data["judge"])
            human_mean = np.mean(data["human"])
            bias = judge_mean - human_mean
            biases["trajectory_type"][traj_type] = {
                "judge_mean": round(judge_mean, 3),
                "human_mean": round(human_mean, 3),
                "bias": round(bias, 3),
                "interpretation": "over-estimates" if bias > 0.1 else "under-estimates" if bias < -0.1 else "calibrated"
            }
    
    # MC disagreement bias
    disagreement_cases = [
        ann for ann in annotations
        if ann.get("mc_label_soft") is not None
        and ann.get("judge_overall_score") is not None
        and abs(ann["mc_label_soft"] - ann["judge_overall_score"]) > 0.3
        and ann.get("human_overall_quality") is not None
    ]
    
    if len(disagreement_cases) >= 10:
        # Who is closer to human: MC or judge?
        mc_errors = [
            abs(ann["mc_label_soft"] - ann["human_overall_quality"])
            for ann in disagreement_cases
        ]
        judge_errors = [
            abs(ann["judge_overall_score"] - ann["human_overall_quality"])
            for ann in disagreement_cases
        ]
        
        biases["disagreement"] = {
            "n_cases": len(disagreement_cases),
            "mc_mae": round(np.mean(mc_errors), 3),
            "judge_mae": round(np.mean(judge_errors), 3),
            "winner": "MC" if np.mean(mc_errors) < np.mean(judge_errors) else "Judge",
            "recommendation": "Trust MC in disagreement cases" if np.mean(mc_errors) < np.mean(judge_errors) 
                            else "Trust judge in disagreement cases"
        }
    
    return biases


# ═══════════════════════════════════════════════════════════════════════════
# INTER-ANNOTATOR RELIABILITY
# ═══════════════════════════════════════════════════════════════════════════

def compute_inter_annotator_reliability(annotations: List[Dict]) -> Dict:
    """
    Compute inter-annotator agreement before using humans as ground truth.
    Requires steps annotated by multiple annotators.
    
    Best practice: Krippendorff's α ≥ 0.67 required for reliable ground truth.
    Reference: https://pmc.ncbi.nlm.nih.gov/articles/PMC12863567/
    """
    from collections import defaultdict
    
    # Group by step identity
    step_annotations = defaultdict(dict)
    for ann in annotations:
        step_id = (ann.get("trajectory_id"), ann.get("step_position"))
        annotator = ann.get("annotator_id", "unknown")
        score = ann.get("human_overall_quality")
        if score is not None:
            step_annotations[step_id][annotator] = score
    
    # Only keep steps with multiple annotators
    multi_annotated = {
        sid: scores for sid, scores in step_annotations.items()
        if len(scores) >= 2
    }
    
    if len(multi_annotated) < 20:
        return {
            "warning": "Less than 20 multiply-annotated steps — IAR unreliable",
            "n_multi_annotated": len(multi_annotated),
            "recommendation": "Have 2 annotators score the same 50 steps minimum"
        }
    
    # Pairwise agreement
    pairs = []
    for step_id, scores in multi_annotated.items():
        annotators = list(scores.keys())
        for i in range(len(annotators)):
            for j in range(i+1, len(annotators)):
                pairs.append((scores[annotators[i]], scores[annotators[j]]))
    
    a_scores, b_scores = zip(*pairs)
    rho, _ = spearmanr(a_scores, b_scores)
    mae = float(np.mean(np.abs(np.array(a_scores) - np.array(b_scores))))
    
    return {
        "n_multiply_annotated_steps": len(multi_annotated),
        "pairwise_spearman": round(float(rho), 3),
        "pairwise_mae": round(float(mae), 3),
        "krippendorff_alpha_approx": round(float(1 - mae / 0.5), 3),
        "reliable": bool(rho >= 0.67),
        "recommendation": (
            "Human annotations are reliable — safe to use as ground truth" 
            if rho >= 0.67 
            else "Human annotations are inconsistent — DO NOT use as ground truth yet. "
                 "Run annotator calibration session first."
        )
    }


# ═══════════════════════════════════════════════════════════════════════════
# RECALIBRATION UTILITIES
# ═══════════════════════════════════════════════════════════════════════════

def generate_few_shot_anchors(annotations: List[Dict], n_per_bin: int = 3) -> Dict:
    """
    Extract anchor examples from human annotations for judge prompt injection.
    Returns high/medium/low quality examples grounded in human labels.
    
    Reference: https://www.langchain.com/articles/llm-as-a-judge
    """
    bins = {"high": [], "medium": [], "low": []}
    
    for ann in annotations:
        score = ann.get("human_overall_quality")
        if score is None:
            continue
        if score >= 0.75:
            bins["high"].append(ann)
        elif score >= 0.40:
            bins["medium"].append(ann)
        else:
            bins["low"].append(ann)
    
    anchors = {}
    for bin_name, bin_steps in bins.items():
        # Pick steps where judge and human AGREE (high confidence anchors)
        agreed = [
            s for s in bin_steps
            if s.get("judge_overall_score") is not None
            and abs(s["judge_overall_score"] - s["human_overall_quality"]) < 0.15
        ]
        if agreed:
            selected = random.sample(agreed, min(n_per_bin, len(agreed)))
            anchors[bin_name] = [{
                "reasoning": s.get("reasoning_normalized", "")[:300],
                "action_type": s.get("action_type"),
                "human_score": s["human_overall_quality"],
                "human_justification": s.get("human_justification", "")
            } for s in selected]
    
    return anchors


def build_calibrated_system_prompt(base_prompt: str, anchors: Dict) -> str:
    """Inject anchor examples into judge system prompt."""
    anchor_text = "\n\n## Calibration Examples\n"
    for quality, examples in anchors.items():
        anchor_text += f"\n### {quality.upper()} QUALITY examples:\n"
        for ex in examples:
            anchor_text += (
                f"- Action: {ex['action_type']}, "
                f"Score: {ex['human_score']:.2f}\n"
                f"  Reasoning snippet: {ex['reasoning'][:150]}\n"
                f"  Why: {ex['human_justification']}\n"
            )
    return base_prompt + anchor_text


# ═══════════════════════════════════════════════════════════════════════════
# CALIBRATION REPORT
# ═══════════════════════════════════════════════════════════════════════════

def generate_calibration_report(
    annotations: List[Dict],
    output_path: Path
):
    """
    Generate comprehensive calibration report.
    
    Includes:
    - Inter-annotator reliability (CRITICAL - validates ground truth)
    - Overall judge-human correlation
    - Per-axis calibration
    - Systematic bias detection
    - Recommendations for PRM training
    - Few-shot anchors for recalibration
    """
    # STEP 1: Check inter-annotator reliability FIRST
    iar_metrics = compute_inter_annotator_reliability(annotations)
    
    if iar_metrics.get("reliable") is False:
        print("\n" + "=" * 60)
        print("  ⚠️  CRITICAL WARNING: UNRELIABLE HUMAN ANNOTATIONS")
        print("=" * 60)
        print(f"\nInter-annotator Spearman: {iar_metrics.get('pairwise_spearman', 'N/A')}")
        print(f"Recommendation: {iar_metrics.get('recommendation')}")
        print("\nThe rest of this report is INVALID until human annotations are consistent.")
        print("=" * 60 + "\n")
    
    # STEP 2: Overall correlation (filter None values)
    paired = [
        (ann["judge_overall_score"], ann["human_overall_quality"])
        for ann in annotations
        if ann.get("human_overall_quality") is not None
        and ann.get("judge_overall_score") is not None
    ]
    
    if len(paired) < 30:
        print(f"Warning: Only {len(paired)} complete pairs — report may be unreliable")
    
    judge_overall, human_overall = zip(*paired) if paired else ([], [])
    
    overall_metrics = compute_correlation_metrics(judge_overall, human_overall, "overall") if paired else {}
    
    # Per-axis calibration
    axis_metrics = analyze_axis_calibration(annotations)
    
    # Bias detection
    biases = detect_systematic_biases(annotations)
    
    # STEP 3: Generate few-shot anchors for recalibration
    anchors = generate_few_shot_anchors(annotations)
    
    # STEP 4: Recommendations (use Spearman rho, not Pearson r)
    recommendations = []
    
    if not overall_metrics:
        recommendations.append({
            "severity": "CRITICAL",
            "issue": "No valid judge-human pairs found",
            "action": "Check that judge_overall_score and human_overall_quality are both present."
        })
    elif overall_metrics["spearman_rho"] < 0.65:
        recommendations.append({
            "severity": "CRITICAL",
            "issue": f"Overall correlation too low (ρ={overall_metrics['spearman_rho']})",
            "action": "DO NOT use judge scores for PRM training. Recalibrate axes or use MC-only."
        })
    elif overall_metrics["spearman_rho"] < 0.75:
        recommendations.append({
            "severity": "WARNING",
            "issue": f"Moderate correlation (ρ={overall_metrics['spearman_rho']})",
            "action": "Use judge scores with caution. Blend with MC (0.6 MC / 0.4 judge)."
        })
    else:
        recommendations.append({
            "severity": "INFO",
            "issue": f"Excellent correlation (ρ={overall_metrics['spearman_rho']})",
            "action": "Judge is well-calibrated. Safe to use in hybrid loss (0.5 MC / 0.5 judge)."
        })
    
    # Check axis-specific issues (use Spearman rho)
    for axis, metrics in axis_metrics.items():
        if metrics["spearman_rho"] < 0.60:
            recommendations.append({
                "severity": "WARNING",
                "issue": f"{axis} poorly calibrated (ρ={metrics['spearman_rho']})",
                "action": f"Downweight {axis} in aggregation or exclude from training."
            })
    
    # Check biases
    for traj_type, bias_data in biases.get("trajectory_type", {}).items():
        if abs(bias_data["bias"]) > 0.15:
            recommendations.append({
                "severity": "WARNING",
                "issue": f"Judge {bias_data['interpretation']} {traj_type} by {abs(bias_data['bias']):.2f}",
                "action": f"Apply bias correction: subtract {bias_data['bias']:.3f} from judge scores for {traj_type}."
            })
    
    # Compile report
    report = {
        "calibration_date": datetime.now().isoformat(),
        "n_annotations": len(annotations),
        "inter_annotator_reliability": iar_metrics,
        "overall_metrics": overall_metrics,
        "axis_metrics": axis_metrics,
        "systematic_biases": biases,
        "recommendations": recommendations,
        "few_shot_anchors": anchors,
        "summary": {
            "judge_quality": overall_metrics.get("interpretation", "N/A"),
            "safe_for_training": bool(overall_metrics.get("spearman_rho", 0) >= 0.65 and iar_metrics.get("reliable", False)),
            "recommended_blend_weight": float(0.5 if overall_metrics.get("spearman_rho", 0) >= 0.75 else 0.4 if overall_metrics.get("spearman_rho", 0) >= 0.65 else 0.3)
        }
    }
    
    # Write report
    with open(output_path, "w") as f:
        json.dump(report, f, indent=2)
    
    # Print summary
    print("\n" + "=" * 60)
    print("  Judge Calibration Report")
    print("=" * 60)
    print(f"\nAnnotations analyzed: {len(annotations)}")
    print(f"Valid pairs: {len(paired)}")
    
    print(f"\nInter-Annotator Reliability:")
    if iar_metrics.get("reliable") is not None:
        print(f"  Spearman ρ: {iar_metrics.get('pairwise_spearman', 'N/A')}")
        print(f"  Status: {'✓ RELIABLE' if iar_metrics.get('reliable') else '✗ UNRELIABLE'}")
    else:
        print(f"  {iar_metrics.get('warning', 'N/A')}")
    
    if overall_metrics:
        print(f"\nOverall Judge-Human Correlation:")
        print(f"  Spearman ρ: {overall_metrics['spearman_rho']} (p={overall_metrics['spearman_p']})")
        print(f"  Pearson r: {overall_metrics['pearson_r']} (p={overall_metrics['pearson_p']})")
        print(f"  MAE: {overall_metrics['mae']}, Bias: {overall_metrics['bias']}")
        print(f"  Interpretation: {overall_metrics['interpretation']}")
    
    if axis_metrics:
        print(f"\nPer-Axis Calibration:")
        for axis, metrics in axis_metrics.items():
            print(f"  {axis}: ρ={metrics['spearman_rho']}, MAE={metrics['mae']}")
    
    if recommendations:
        print(f"\nRecommendations:")
        for rec in recommendations:
            print(f"  [{rec['severity']}] {rec['issue']}")
            print(f"    → {rec['action']}")
    
    if anchors:
        print(f"\nFew-Shot Anchors Generated:")
        for quality, examples in anchors.items():
            print(f"  {quality}: {len(examples)} examples")
    
    print(f"\nFull report saved to: {output_path}")
    print("=" * 60 + "\n")


# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(description="Judge Calibration Module")
    subparsers = parser.add_subparsers(dest="command", required=True)
    
    # Sample command
    sample_parser = subparsers.add_parser("sample", help="Sample steps for human annotation")
    sample_parser.add_argument("--input", type=Path, required=True)
    sample_parser.add_argument("--output", type=Path, required=True)
    sample_parser.add_argument("--n_samples", type=int, default=300)
    sample_parser.add_argument("--seed", type=int, default=42)
    
    # Analyze command
    analyze_parser = subparsers.add_parser("analyze", help="Analyze calibration metrics")
    analyze_parser.add_argument("--judge_labels", type=Path, required=True)
    analyze_parser.add_argument("--human_labels", type=Path, required=True)
    analyze_parser.add_argument("--output", type=Path, required=True)
    
    args = parser.parse_args()
    
    if args.command == "sample":
        random.seed(args.seed)
        
        # Load steps
        steps = []
        with open(args.input) as f:
            for line in f:
                if line.strip():
                    steps.append(json.loads(line))
        
        print(f"Loaded {len(steps)} steps")
        
        # Sample
        sampled = stratified_sample_for_annotation(steps, args.n_samples)
        
        print(f"Sampled {len(sampled)} steps for annotation")
        
        # Create annotation templates
        templates = [create_annotation_template(step) for step in sampled]
        
        # Write output
        with open(args.output, "w") as f:
            for template in templates:
                f.write(json.dumps(template) + "\n")
        
        print(f"Annotation batch saved to: {args.output}")
        print("\nNext step: Annotate using annotation_interface.html")
    
    elif args.command == "analyze":
        # Load annotations
        annotations = []
        with open(args.human_labels) as f:
            for line in f:
                if line.strip():
                    annotations.append(json.loads(line))
        
        # Filter to completed annotations
        completed = [
            ann for ann in annotations
            if ann.get("human_overall_quality") is not None
        ]
        
        print(f"Loaded {len(completed)} completed annotations")
        
        if len(completed) < 50:
            print("Warning: Less than 50 annotations. Results may be unreliable.")
        
        # Generate report
        generate_calibration_report(completed, args.output)


if __name__ == "__main__":
    main()
