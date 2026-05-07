#!/usr/bin/env python3
"""
Consensus Labeling Module for t3_cpp

Blends MC, judge, and reward signals with explicit conflict resolution rules.
Produces final PRM training labels with confidence scores.

Usage:
    python consensus_labeling.py \
        --input labeled_steps_with_judge.jsonl \
        --calibration calibration_report.json \
        --output prm_training_labels.jsonl \
        --strategy adaptive
"""

import argparse
import json
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import numpy as np

from code_and_train_ds.t3_cpp.core.llm_judge_framework.shared_constants import (
    normalize_to_prm_range,
    judge_to_consensus_scale,
    MC_ADVANTAGE_RANGE,
    TrajectoryType
)

logger = logging.getLogger("consensus")


# ═══════════════════════════════════════════════════════════════════════════
# PLAUSIBLE BUT WRONG DETECTION
# ═══════════════════════════════════════════════════════════════════════════

def detect_plausible_but_wrong(step: Dict) -> Dict:
    """
    Detect when judge is fooled by good reasoning but bad outcome.
    
    This is the "plausible but wrong" problem where:
    - Reasoning sounds logical and well-structured
    - Code looks syntactically correct
    - But observation shows failure (compile error, timeout, test failure)
    - Judge gives high score despite bad outcome
    
    Args:
        step: Step dict with judge scores, MC labels, and observation
    
    Returns:
        Dict with detection results:
        - is_plausible_but_wrong: bool
        - confidence: float (0.0-1.0)
        - reason: str
        - recommended_action: str
    """
    judge_score = step.get("judge_overall_score")
    mc_soft = step.get("mc_label_soft", 0.5)
    observation = step.get("observation", {})
    
    # Early exit if missing data
    if judge_score is None:
        return {
            "is_plausible_but_wrong": False,
            "confidence": 0.0,
            "reason": "no_judge_score",
            "recommended_action": "none"
        }
    
    # Check observation for failure signals
    obs_status = observation.get("status", "unknown")
    exit_code = observation.get("exit_code")
    stderr = str(observation.get("stderr", ""))
    error_type = step.get("error_type", "")
    
    has_error = (
        obs_status in ["error", "timeout", "failed", "compile_error"] or
        (exit_code is not None and exit_code != 0) or
        len(stderr) > 50 or
        error_type in ["compile_error", "runtime_error", "timeout"]
    )
    
    # Check for disagreement between judge and MC
    judge_optimistic = judge_score > 0.5  # Judge thinks it's good
    mc_pessimistic = mc_soft < 0.35  # MC knows it failed
    disagreement = abs(judge_score - mc_soft)
    
    # Detection logic
    if judge_optimistic and mc_pessimistic and has_error:
        # Strong signal: all three indicators present
        confidence = min(1.0, disagreement + 0.3)
        return {
            "is_plausible_but_wrong": True,
            "confidence": round(confidence, 3),
            "reason": f"judge_optimistic({judge_score:.2f})_mc_pessimistic({mc_soft:.2f})_error({obs_status})",
            "recommended_action": "trust_mc_heavily",
            "disagreement": round(disagreement, 3),
            "obs_status": obs_status,
            "error_type": error_type
        }
    
    elif judge_optimistic and has_error:
        # Medium signal: judge optimistic despite error (no MC confirmation)
        confidence = 0.6
        return {
            "is_plausible_but_wrong": True,
            "confidence": confidence,
            "reason": f"judge_optimistic({judge_score:.2f})_error({obs_status})_no_mc_confirmation",
            "recommended_action": "flag_for_review",
            "obs_status": obs_status,
            "error_type": error_type
        }
    
    elif judge_optimistic and mc_pessimistic:
        # Weak signal: disagreement but no clear error in observation
        confidence = 0.4
        return {
            "is_plausible_but_wrong": True,
            "confidence": confidence,
            "reason": f"judge_mc_disagreement({disagreement:.2f})_no_clear_error",
            "recommended_action": "monitor",
            "disagreement": round(disagreement, 3)
        }
    
    else:
        # No plausible-but-wrong detected
        return {
            "is_plausible_but_wrong": False,
            "confidence": 0.0,
            "reason": "no_indicators",
            "recommended_action": "none"
        }


# ═══════════════════════════════════════════════════════════════════════════
# NORMALIZATION UTILITIES
# ═══════════════════════════════════════════════════════════════════════════

def normalize_mc_advantage(mc_adv: float, 
                           source_range: Tuple[float, float] = (-1.0, 1.0)) -> float:
    """Normalize MC advantage to [-1, 1] range."""
    return normalize_to_prm_range(mc_adv, source_range)


# ═══════════════════════════════════════════════════════════════════════════
# CONSENSUS STRATEGIES
# ═══════════════════════════════════════════════════════════════════════════

class ConsensusStrategy:
    """Base class for consensus strategies."""
    
    def __init__(self, calibration: Dict = None):
        """
        Args:
            calibration: Calibration report from calibrate_judge.py
        """
        self.calibration = calibration or {}
        # Use Spearman ρ (better for non-linear distributions), fallback to Pearson r
        self.judge_quality = self.calibration.get("overall_metrics", {}).get(
            "spearman_rho",
            self.calibration.get("overall_metrics", {}).get("pearson_r", 0.7)
        )
        self.recommended_weight = self.calibration.get("summary", {}).get("recommended_blend_weight", 0.5)
        # MC advantage range for normalization
        self.mc_advantage_range = self.calibration.get("mc_advantage_range", (-1.0, 1.0))
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        """
        Compute consensus label for a step.
        
        Returns:
            (consensus_label, confidence, metadata) or None if step should be dropped
        """
        raise NotImplementedError


class MCOnlyStrategy(ConsensusStrategy):
    """Use MC labels only (baseline)."""
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        mc_adv = step.get("mc_advantage_normalized", step.get("mc_advantage", 0.0))
        mc_reliable = step.get("mc_reliable", False)
        mc_zero_data = step.get("mc_zero_data", False)
        
        # Drop steps with no MC data
        if mc_zero_data:
            return None
        
        # Normalize MC advantage to [-1, 1]
        mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
        
        confidence = 1.0 if mc_reliable else 0.4
        
        return mc_adv_norm, confidence, {
            "strategy": "mc_only",
            "source": "mc_advantage_normalized"
        }


class JudgeOnlyStrategy(ConsensusStrategy):
    """Use judge labels only."""
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        judge_score = step.get("judge_overall_score")
        
        # Drop if judge score unavailable
        if judge_score is None:
            return None
        
        # Normalize to [-1, 1] range (consistent with other strategies)
        judge_normalized = judge_to_consensus_scale(judge_score)
        
        # Confidence based on calibration quality
        confidence = self.judge_quality
        
        return judge_normalized, confidence, {
            "strategy": "judge_only",
            "source": "judge_overall_score"
        }


class FixedBlendStrategy(ConsensusStrategy):
    """Fixed weighted blend of MC and judge."""
    
    def __init__(self, calibration: Dict = None, alpha: float = 0.7):
        """
        Args:
            alpha: Weight for MC (1-alpha for judge)
        """
        super().__init__(calibration)
        self.alpha = alpha
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        mc_adv = step.get("mc_advantage_normalized", step.get("mc_advantage", 0.0))
        judge_score = step.get("judge_overall_score")
        
        # If judge unavailable, fall back to MC-only
        if judge_score is None:
            mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
            mc_reliable = step.get("mc_reliable", False)
            return mc_adv_norm, (1.0 if mc_reliable else 0.3), {
                "strategy": "fixed_blend",
                "fallback": "mc_only_judge_unavailable",
                "alpha": 1.0
            }
        
        # Normalize both to [-1, 1] range
        mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
        judge_normalized = judge_to_consensus_scale(judge_score)
        
        consensus = self.alpha * mc_adv_norm + (1 - self.alpha) * judge_normalized
        
        # Confidence is min of MC and judge confidence
        mc_reliable = step.get("mc_reliable", False)
        mc_conf = 1.0 if mc_reliable else 0.3
        judge_conf = self.judge_quality
        
        confidence = min(mc_conf, judge_conf)
        
        return consensus, confidence, {
            "strategy": "fixed_blend",
            "alpha": self.alpha,
            "mc_contribution": self.alpha * mc_adv_norm,
            "judge_contribution": (1 - self.alpha) * judge_normalized
        }


class AdaptiveStrategy(ConsensusStrategy):
    """
    Adaptive blending based on MC reliability and agreement.
    
    Rules:
    1. If MC unreliable (mc_reliable=False or mc_zero_data=True), trust judge
    2. If MC and judge agree (within 0.2), blend with rollout weighting
    3. If MC and judge disagree strongly (>0.3), use calibration to decide
    4. If moderate disagreement (0.2-0.3), use trajectory-type-specific blending
    """
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        mc_adv = step.get("mc_advantage_normalized", step.get("mc_advantage", 0.0))
        mc_soft = step.get("mc_label_soft", 0.5)
        mc_reliable = step.get("mc_reliable", False)
        mc_zero_data = step.get("mc_zero_data", False)
        mc_n_rollouts = max(1, step.get("mc_n_rollouts", 1))  # Ensure >= 1
        
        judge_score = step.get("judge_overall_score")
        
        # If judge unavailable, use MC-only
        if judge_score is None:
            if mc_zero_data:
                return None  # No data at all
            mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
            return mc_adv_norm, (1.0 if mc_reliable else 0.4), {
                "strategy": "adaptive",
                "rule": "judge_unavailable",
                "alpha": 1.0
            }
        
        judge_normalized = judge_to_consensus_scale(judge_score)
        
        # Normalize MC advantage to [-1, 1]
        mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
        
        traj_type = step.get("judge_trajectory_type", TrajectoryType.UNKNOWN.value)
        
        # Rule 1: MC unreliable → trust judge
        if not mc_reliable or mc_zero_data:
            return judge_normalized, self.judge_quality, {
                "strategy": "adaptive",
                "rule": "mc_unreliable",
                "reason": "mc_zero_data" if mc_zero_data else "mc_not_reliable",
                "alpha": 0.0
            }
        
        # Calculate disagreement
        disagreement = abs(mc_soft - judge_score)
        
        # Rule 2: Strong agreement (≤0.2) → blend with MC rollout weighting
        if disagreement <= 0.2:
            # Higher rollout count → trust MC more
            alpha = min(0.8, 0.5 + 0.1 * np.log1p(mc_n_rollouts - 1))
            consensus = alpha * mc_adv_norm + (1 - alpha) * judge_normalized
            return consensus, 0.9, {
                "strategy": "adaptive",
                "rule": "agreement",
                "disagreement": round(disagreement, 3),
                "alpha": round(alpha, 3),
                "mc_n_rollouts": mc_n_rollouts
            }
        
        # Rule 3: Strong disagreement (>0.3) → use calibration
        if disagreement > 0.3:
            # Check for "plausible but wrong" pattern
            pbw_detection = detect_plausible_but_wrong(step)
            
            if pbw_detection["is_plausible_but_wrong"] and pbw_detection["confidence"] > 0.7:
                # High confidence plausible-but-wrong: trust MC heavily
                alpha = 0.9  # 90% MC, 10% judge
                consensus = alpha * mc_adv_norm + (1 - alpha) * judge_normalized
                return consensus, 0.7, {
                    "strategy": "adaptive",
                    "rule": "plausible_but_wrong_detected",
                    "disagreement": round(disagreement, 3),
                    "pbw_confidence": pbw_detection["confidence"],
                    "pbw_reason": pbw_detection["reason"],
                    "alpha": alpha
                }
            
            # Standard strong disagreement handling
            disagreement_bias = self.calibration.get("systematic_biases", {}).get("disagreement", {})
            winner = disagreement_bias.get("winner", "MC")
            
            alpha = 0.8 if winner == "MC" else 0.3
            
            consensus = alpha * mc_adv_norm + (1 - alpha) * judge_normalized
            return consensus, 0.6, {
                "strategy": "adaptive",
                "rule": "strong_disagreement",
                "disagreement": round(disagreement, 3),
                "calibration_winner": winner,
                "alpha": alpha
            }
        
        # Rule 4: Moderate disagreement (0.2 < disagreement ≤ 0.3) → trajectory-type-specific blending
        type_alphas = {
            TrajectoryType.HIGH_CONF_FAILURE.value: 0.4,  # Trust judge's correctness auditor
            TrajectoryType.RECOVERY_SUCCESS.value: 0.5,
            TrajectoryType.TESTED_FAILURE.value: 0.5,
        }
        alpha = type_alphas.get(traj_type, 0.6)  # Default: trust MC slightly more
        
        consensus = alpha * mc_adv_norm + (1 - alpha) * judge_normalized
        
        return consensus, 0.8, {
            "strategy": "adaptive",
            "rule": "trajectory_type_adjusted",
            "trajectory_type": traj_type,
            "disagreement": round(disagreement, 3),
            "alpha": alpha
        }


class ConsensusFilteringStrategy(ConsensusStrategy):
    """
    Strict consensus: only keep steps where MC and judge agree.
    Drops conflicting steps from training.
    """
    
    def __init__(self, calibration: Dict = None, agreement_threshold: float = 0.25):
        super().__init__(calibration)
        self.agreement_threshold = agreement_threshold
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[float, float, Dict]]:
        mc_soft = step.get("mc_label_soft", 0.5)
        judge_score = step.get("judge_overall_score")
        
        # Drop if judge unavailable
        if judge_score is None:
            return None
        
        disagreement = abs(mc_soft - judge_score)
        
        # Explicit drop signal: return None
        if disagreement > self.agreement_threshold:
            return None
        
        # Agreement: blend equally
        mc_adv = step.get("mc_advantage_normalized", step.get("mc_advantage", 0.0))
        mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
        judge_normalized = judge_to_consensus_scale(judge_score)
        
        consensus = 0.5 * mc_adv_norm + 0.5 * judge_normalized
        
        return consensus, 1.0, {
            "strategy": "consensus_filtering",
            "rule": "kept",
            "disagreement": round(disagreement, 3)
        }


class MultiTaskStrategy(ConsensusStrategy):
    """
    Multi-task labels: keep MC, judge, and reward as separate targets.
    No blending - let the model learn to weight them.
    """
    
    def compute_consensus_label(self, step: Dict) -> Optional[Tuple[Dict, float, Dict]]:
        """
        Returns:
            (multi_task_labels_dict, confidence, metadata) or None if dropped
        """
        mc_adv = step.get("mc_advantage_normalized", step.get("mc_advantage", 0.0))
        judge_score = step.get("judge_overall_score")
        step_reward = step.get("step_reward", 0.0)
        
        # Normalize all to [-1, 1] range
        mc_adv_norm = normalize_to_prm_range(mc_adv, self.mc_advantage_range)
        judge_normalized = judge_to_consensus_scale(judge_score) if judge_score is not None else 0.0
        reward_normalized = max(-1.0, min(1.0, float(step_reward)))  # Clamp to [-1, 1]
        
        labels = {
            "mc_advantage": mc_adv_norm,
            "judge_score": judge_normalized,
            "step_reward": reward_normalized,
        }
        
        # Add flag if judge unavailable
        if judge_score is None:
            labels["judge_score_available"] = False
        
        # Confidence based on MC reliability
        mc_reliable = step.get("mc_reliable", False)
        confidence = 0.9 if mc_reliable else 0.5
        
        return labels, confidence, {
            "strategy": "multi_task",
            "rule": "separate_targets",
            "judge_available": judge_score is not None
        }


# ═══════════════════════════════════════════════════════════════════════════
# STRATEGY FACTORY
# ═══════════════════════════════════════════════════════════════════════════

STRATEGIES = {
    "mc_only": MCOnlyStrategy,
    "judge_only": JudgeOnlyStrategy,
    "fixed_blend": FixedBlendStrategy,
    "adaptive": AdaptiveStrategy,
    "consensus_filtering": ConsensusFilteringStrategy,
    "multi_task": MultiTaskStrategy,
}


def create_strategy(strategy_name: str, calibration: Dict = None, **kwargs) -> ConsensusStrategy:
    """Create consensus strategy instance."""
    if strategy_name not in STRATEGIES:
        raise ValueError(f"Unknown strategy: {strategy_name}. Choose from {list(STRATEGIES.keys())}")
    
    return STRATEGIES[strategy_name](calibration, **kwargs)


# ═══════════════════════════════════════════════════════════════════════════
# CONSENSUS LABELING
# ═══════════════════════════════════════════════════════════════════════════

def apply_consensus_labeling(
    steps: List[Dict],
    strategy: ConsensusStrategy,
    min_confidence: float = 0.0
) -> List[Dict]:
    """
    Apply consensus labeling to all steps.
    
    Args:
        steps: Steps with MC and judge labels
        strategy: Consensus strategy
        min_confidence: Minimum confidence to keep step (for filtering)
    
    Returns:
        Steps with consensus labels added
    """
    labeled_steps = []
    dropped_count = 0
    
    for step in steps:
        # Compute consensus
        result = strategy.compute_consensus_label(step)
        
        # Handle explicit drop signal (None)
        if result is None:
            dropped_count += 1
            continue
        
        if isinstance(result[0], dict):
            # Multi-task strategy
            consensus_labels, confidence, metadata = result
            step["prm_labels"] = consensus_labels
        else:
            # Single-target strategy
            consensus_label, confidence, metadata = result
            step["prm_label"] = round(float(consensus_label), 4)
        
        step["prm_confidence"] = confidence
        step["prm_metadata"] = metadata
        
        # Detect "plausible but wrong" pattern
        pbw_detection = detect_plausible_but_wrong(step)
        step["plausible_but_wrong"] = pbw_detection
        
        # Flag high-confidence plausible-but-wrong cases for review
        if pbw_detection["is_plausible_but_wrong"] and pbw_detection["confidence"] > 0.7:
            step["needs_human_review"] = True
            step["review_reason"] = "plausible_but_wrong_high_confidence"
        
        # Filter by confidence
        if confidence >= min_confidence:
            labeled_steps.append(step)
        else:
            dropped_count += 1
    
    logger.info(f"Consensus labeling complete: {len(labeled_steps)} kept, {dropped_count} dropped")
    
    return labeled_steps


# ═══════════════════════════════════════════════════════════════════════════
# TRAJECTORY CONSISTENCY
# ═══════════════════════════════════════════════════════════════════════════

def enforce_trajectory_consistency(
    steps: List[Dict], 
    max_single_step_swing: float = 0.6
) -> List[Dict]:
    """
    Flag steps where label swings implausibly relative to neighbors.
    Groups by trajectory first to avoid cross-trajectory comparisons.
    Does not modify labels — only sets warning flags for downstream filtering.
    
    Args:
        steps: Steps with prm_label
        max_single_step_swing: Maximum allowed label change between consecutive steps
    
    Returns:
        Steps with swing warnings added
    """
    from collections import defaultdict
    
    # Group by trajectory
    by_trajectory = defaultdict(list)
    for step in steps:
        traj_id = step.get("trajectory_id")
        if traj_id:
            by_trajectory[traj_id].append(step)
    
    # Check consistency within each trajectory
    swing_count = 0
    for traj_id, traj_steps in by_trajectory.items():
        # Sort by step position
        traj_steps.sort(key=lambda s: s.get("step_position", 0))
        
        for i in range(1, len(traj_steps)):
            curr_label = traj_steps[i].get("prm_label")
            prev_label = traj_steps[i-1].get("prm_label")
            
            if curr_label is not None and prev_label is not None:
                swing = abs(curr_label - prev_label)
                if swing > max_single_step_swing:
                    traj_steps[i]["prm_label_swing_warning"] = True
                    traj_steps[i]["prm_label_swing_delta"] = round(swing, 3)
                    swing_count += 1
    
    if swing_count > 0:
        logger.warning(f"Found {swing_count} steps with large label swings (>{max_single_step_swing})")
    
    return steps


# ═══════════════════════════════════════════════════════════════════════════
# STATISTICS
# ═══════════════════════════════════════════════════════════════════════════

def print_consensus_statistics(steps: List[Dict]):
    """Print statistics about consensus labeling."""
    print("\n" + "=" * 60)
    print("  Consensus Labeling Statistics")
    print("=" * 60)
    
    if not steps:
        print("\n⚠️  No steps remaining after consensus labeling!")
        print("All steps were dropped. Check strategy and min_confidence settings.")
        print("=" * 60 + "\n")
        return
    
    # Overall
    print(f"\nTotal steps: {len(steps)}")
    
    # Confidence distribution
    confidences = [s["prm_confidence"] for s in steps]
    print(f"\nConfidence distribution:")
    print(f"  Mean: {np.mean(confidences):.3f}")
    print(f"  Median: {np.median(confidences):.3f}")
    print(f"  Min: {np.min(confidences):.3f}, Max: {np.max(confidences):.3f}")
    
    # High confidence samples
    high_conf = sum(1 for c in confidences if c >= 0.8)
    print(f"  High confidence (≥0.8): {high_conf} / {len(steps)} ({100*high_conf/len(steps):.1f}%)")
    
    # Strategy breakdown
    from collections import Counter
    strategies = Counter(s["prm_metadata"]["strategy"] for s in steps)
    print(f"\nStrategy usage:")
    for strategy, count in strategies.most_common():
        print(f"  {strategy}: {count} ({100*count/len(steps):.1f}%)")
    
    # Rule breakdown (for adaptive)
    rules = Counter(s["prm_metadata"].get("rule", "N/A") for s in steps)
    print(f"\nRule usage:")
    for rule, count in rules.most_common():
        print(f"  {rule}: {count} ({100*count/len(steps):.1f}%)")
    
    # Label distribution
    if steps[0].get("prm_labels"):
        # Multi-task strategy
        print(f"\nMulti-task label distributions:")
        for key in steps[0]["prm_labels"].keys():
            vals = [s["prm_labels"][key] for s in steps if "prm_labels" in s]
            pos = sum(1 for v in vals if v > 0)
            neg = sum(1 for v in vals if v < 0)
            print(f"  {key}: mean={np.mean(vals):.3f}, std={np.std(vals):.3f}")
            print(f"    Positive: {pos} ({100*pos/len(vals):.1f}%), Negative: {neg} ({100*neg/len(vals):.1f}%)")
    elif any("prm_label" in s for s in steps):
        # Single-target strategy
        labels = [s["prm_label"] for s in steps if "prm_label" in s]
        pos = sum(1 for l in labels if l > 0)
        neg = sum(1 for l in labels if l < 0)
        neutral = len(labels) - pos - neg
        print(f"\nLabel distribution:")
        print(f"  Mean: {np.mean(labels):.3f}")
        print(f"  Std: {np.std(labels):.3f}")
        print(f"  Positive (>0): {pos} ({100*pos/len(labels):.1f}%)")
        print(f"  Negative (<0): {neg} ({100*neg/len(labels):.1f}%)")
        print(f"  Neutral (=0): {neutral} ({100*neutral/len(labels):.1f}%)")
    
    # Trajectory consistency warnings
    swing_warnings = sum(1 for s in steps if s.get("prm_label_swing_warning"))
    if swing_warnings > 0:
        print(f"\nTrajectory consistency warnings:")
        print(f"  Steps with large label swings: {swing_warnings} ({100*swing_warnings/len(steps):.1f}%)")
        max_swing = max((s.get("prm_label_swing_delta", 0) for s in steps), default=0)
        print(f"  Maximum swing: {max_swing:.3f}")
    
    # Plausible-but-wrong detection statistics
    pbw_detected = sum(1 for s in steps if s.get("plausible_but_wrong", {}).get("is_plausible_but_wrong", False))
    if pbw_detected > 0:
        print(f"\nPlausible-but-Wrong Detection:")
        print(f"  Total detected: {pbw_detected} ({100*pbw_detected/len(steps):.1f}%)")
        
        high_conf_pbw = sum(1 for s in steps 
                           if s.get("plausible_but_wrong", {}).get("is_plausible_but_wrong", False) 
                           and s.get("plausible_but_wrong", {}).get("confidence", 0) > 0.7)
        print(f"  High confidence (>0.7): {high_conf_pbw} ({100*high_conf_pbw/len(steps):.1f}%)")
        
        needs_review = sum(1 for s in steps if s.get("needs_human_review", False))
        print(f"  Flagged for human review: {needs_review}")
        
        # Breakdown by reason
        from collections import Counter
        pbw_reasons = Counter(
            s.get("plausible_but_wrong", {}).get("reason", "unknown")
            for s in steps
            if s.get("plausible_but_wrong", {}).get("is_plausible_but_wrong", False)
        )
        print(f"  Detection reasons:")
        for reason, count in pbw_reasons.most_common(5):
            print(f"    {reason}: {count}")
    
    print("=" * 60 + "\n")


# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(description="Consensus Labeling Module")
    parser.add_argument("--input", type=Path, required=True, help="Input JSONL with MC and judge labels")
    parser.add_argument("--output", type=Path, required=True, help="Output JSONL with consensus labels")
    parser.add_argument(
        "--strategy", type=str, default="adaptive",
        choices=list(STRATEGIES.keys()),
        help="Consensus strategy"
    )
    parser.add_argument("--calibration", type=Path, help="Calibration report JSON")
    parser.add_argument("--min_confidence", type=float, default=0.0, help="Minimum confidence to keep step")
    parser.add_argument("--alpha", type=float, default=0.7, help="Alpha for fixed_blend strategy")
    parser.add_argument("--verbose", action="store_true")
    
    args = parser.parse_args()
    
    logging.basicConfig(
        level=logging.INFO if args.verbose else logging.WARNING,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s"
    )
    
    # Load calibration
    calibration = None
    if args.calibration and args.calibration.exists():
        with open(args.calibration) as f:
            calibration = json.load(f)
        logger.info(f"Loaded calibration from {args.calibration}")
    else:
        logger.warning("No calibration provided - using default weights")
    
    # Load steps
    steps = []
    with open(args.input) as f:
        for line in f:
            if line.strip():
                steps.append(json.loads(line))
    
    logger.info(f"Loaded {len(steps)} steps")
    
    # Create strategy
    strategy_kwargs = {}
    if args.strategy == "fixed_blend":
        strategy_kwargs["alpha"] = args.alpha
    
    strategy = create_strategy(args.strategy, calibration, **strategy_kwargs)
    logger.info(f"Using strategy: {args.strategy}")
    
    # Apply consensus labeling
    labeled_steps = apply_consensus_labeling(steps, strategy, args.min_confidence)
    
    # Enforce trajectory consistency (adds warning flags)
    labeled_steps = enforce_trajectory_consistency(labeled_steps)
    
    # Write output
    with open(args.output, "w") as f:
        for step in labeled_steps:
            f.write(json.dumps(step, default=str) + "\n")
    
    logger.info(f"Wrote {len(labeled_steps)} labeled steps to {args.output}")
    
    # Print statistics
    print_consensus_statistics(labeled_steps)
    
    # Recommendations
    print("\nRecommendations for PRM Training:")
    print(f"1. Filter: prm_confidence >= 0.5 ({sum(1 for s in labeled_steps if s['prm_confidence'] >= 0.5)} steps)")
    print(f"2. Loss: Use prm_label as target")
    print(f"3. Weighting: Weight samples by prm_confidence")
    print(f"4. Validation: Track correlation with final outcome")
    
    # Plausible-but-wrong recommendations
    pbw_count = sum(1 for s in labeled_steps if s.get("plausible_but_wrong", {}).get("is_plausible_but_wrong", False))
    if pbw_count > 0:
        print(f"\n5. Plausible-but-Wrong Handling:")
        print(f"   - {pbw_count} steps detected with judge bias")
        print(f"   - These steps use higher MC weight (90%) in consensus")
        print(f"   - Consider manual review of high-confidence cases")
        print(f"   - Filter: plausible_but_wrong.confidence < 0.8 for cleaner training data")


if __name__ == "__main__":
    main()
