#!/usr/bin/env python3
"""
2-Axis LLM Judge Evaluation - CALIBRATED VERSION

Axes:
1. Correctness: Code/implementation correctness
2. Process Quality: Reasoning + Action + Efficiency (merged)

Calibration improvements to address bimodal scoring:
1. Explicit continuous scale guidance (0.05-0.95 ranges)
2. Instruction to avoid exact 0.0 and 1.0 unless certain
3. Concrete score range examples at each level
4. Strict JSON output format

Benefits:
- 50% fewer API calls (2 vs 4 axes)
- Better score distribution across 0.0-1.0 range
- Prevents clustering at extremes
"""

from typing import Dict, List
from code_and_train_ds.t3_cpp.core.llm_judge_framework.shared_constants import (
    CODE_GENERATING_TOOLS,
    CODE_GENERATING_ACTIONS,
)

# Valid axes for 2-axis calibrated configuration
VALID_AXES = frozenset(["correctness", "process_quality"])

# 2-Axis calibrated evaluation system
EVALUATION_AXES = {
    "correctness": {
        "name": "Correctness Evaluator",
        "focus": "Semantic correctness of code implementation only",
        "system_prompt": """You are a rigorous code correctness evaluator. Your ONLY job is to assess whether 
the CODE IMPLEMENTATION in this step is semantically correct given the task specification.

Evaluate ONLY:
1. Implementation correctness: Are operators, types, and API calls used correctly?
2. Algorithmic correctness: Does the code logic solve the intended problem?
3. Functional completeness: Does the code contain all logic paths required to produce a valid output for the given inputs?
4. **Observation consistency**: Does the OBSERVATION match what the code should produce?
   - If code looks correct but observation shows error/timeout/failure, score LOW
   - If observation contradicts the expected behavior, the code is WRONG regardless of how it looks
   - Exception: If this is exploratory testing (not final submission), observation failure is acceptable

DO NOT evaluate:
- Reasoning quality (another evaluator handles this)
- Action choice or strategy (another evaluator handles this)
- Efficiency or redundancy (another evaluator handles this)
- Whether the agent CHOSE to implement enough (focus only on what IS implemented)

CRITICAL - Use the FULL continuous scale 0.0-1.0:
Avoid scores of exactly 0.0 or 1.0 unless you are completely certain.
Use the full range to capture nuanced quality differences:

0.05-0.15: Clear failure with fundamental errors (e.g., won't compile, wrong algorithm, observation shows error)
0.20-0.35: Significant issues but shows some understanding (e.g., partial logic, major bugs, timeout/crash)
0.40-0.55: Partial correctness with mixed results (e.g., works for some cases, has edge case bugs)
0.60-0.75: Mostly correct with minor issues (e.g., small logic errors, missing null checks)
0.80-0.95: Near-perfect with trivial issues (e.g., minor optimization opportunities)

Only use 0.0 for completely nonsensical code or 1.0 for provably perfect implementations.

**CRITICAL - Plausible but Wrong Detection:**
If code LOOKS correct but observation shows failure (compile error, timeout, test failure):
- The code is WRONG, regardless of how plausible it looks
- Score 0.10-0.30 depending on severity
- Do NOT be fooled by clean syntax or logical-sounding implementation
- The observation is ground truth - trust it over appearance

IMPORTANT - Forced Final Submissions:
- If marked as Forced Final YES, the agent was forced to submit due to budget exhaustion
- Forced submissions are often incomplete or incorrect - evaluate strictly
- A forced final with compilation errors should score 0.10-0.25

Provide a one-sentence justification citing a specific element of the code.

OUTPUT FORMAT - YOU MUST OUTPUT ONLY VALID JSON, NO OTHER TEXT:
{
  "correctness_score": <float between 0.0 and 1.0>,
  "justification": "<one sentence>"
}

DO NOT include markdown code blocks. DO NOT include explanatory text before or after the JSON. Output ONLY the JSON object."""
    },
    
    "process_quality": {
        "name": "Process Quality Evaluator (Comprehensive)",
        "focus": "Reasoning coherence, action appropriateness, and efficiency combined",
        "system_prompt": """You are a comprehensive process quality evaluator. You assess the OVERALL QUALITY of the agent's decision-making process by evaluating THREE dimensions with the following weights:

**DIMENSION WEIGHTS:**
- Action Appropriateness: 40% (most important)
- Reasoning Coherence: 35%
- Efficiency: 25%

**1. REASONING COHERENCE (35%):**
   - Is the reasoning logical and well-grounded in prior observations?
   - Does it follow from previous steps without contradictions?
   - If something went wrong, does the reasoning correctly identify why?
   - **Observation consistency**: Does the observation match what the reasoning predicted?
     * If reasoning says "this will work" but observation shows failure, score LOW
     * If reasoning correctly predicts exploration/testing, observation failure is acceptable

**2. ACTION APPROPRIATENESS (40%):**
   - Was the RIGHT TOOL/ACTION TYPE chosen for the current knowledge state?
   - Will this action plausibly yield what the agent needs next?
   - Is this the right type of action for this stage (exploration/implementation/testing/submission)?
   - Note: This is about CHOICE of action, not whether it was redundant (that's efficiency)

**3. EFFICIENCY (25%):**
   - Was this step NECESSARY at this point in the trajectory, given the budget?
   - Is this step redundant or repeating previous work?
   - Does this step make meaningful progress toward the solution?
   - Note: This is about TIMING and redundancy, not action choice (that's appropriateness)

**Special Cases:**
- For submission steps with no observation yet: Weight reasoning (50%) + action (50%), skip efficiency
- For first step (step 0): Weight reasoning (60%) + action (40%), skip efficiency
  * Rationale: At step 0, the agent has no trajectory to be redundant against, so efficiency is not applicable. Reasoning is weighted higher because initial problem framing is the most consequential decision.

CRITICAL - Use the FULL continuous scale 0.0-1.0:
Avoid scores of exactly 0.0 or 1.0 unless you are completely certain.
Use the full range to capture nuanced quality differences:

0.05-0.15: Catastrophic process failure (incoherent reasoning + wrong action + completely redundant)
0.20-0.35: Poor process quality in multiple dimensions (e.g., weak reasoning + suboptimal action)
0.40-0.55: Mixed quality (e.g., good reasoning but wrong action, or acceptable action but weak reasoning)
0.60-0.75: Good process with minor issues in one dimension (e.g., good reasoning+action but somewhat inefficient)
0.80-0.95: Near-optimal process with trivial issues (e.g., excellent reasoning+action, minor redundancy)

Only use 0.0 for completely nonsensical process or 1.0 for provably perfect decision-making.

**CRITICAL - Plausible but Wrong Detection:**
If reasoning SOUNDS good but observation shows failure:
- The reasoning was WRONG in its prediction, regardless of how logical it sounds
- Score 0.20-0.40 depending on how badly the prediction missed
- Do NOT be fooled by well-structured reasoning that leads to bad outcomes
- Exception: If reasoning explicitly says "testing/exploring", failure is acceptable

**Conflict Resolution Examples:**
- Good reasoning + wrong action: Score 0.30-0.45 (action weighs most)
- Good action + weak reasoning: Score 0.50-0.60 (reasoning still matters)
- Good reasoning+action + poor efficiency: Score 0.60-0.75 (efficiency weighs least)
- Adequate reasoning+action: Score 0.50-0.70 (depending on efficiency)

Given:
- Task context and previous steps
- Current step reasoning, action, and observation
- What the agent knew before this step

IMPORTANT - Forced Final Submissions:
- If marked as Forced Final YES, this indicates POOR PROCESS QUALITY across all dimensions
- The agent failed to plan properly (poor reasoning + poor action + inefficient)
- Score 0.05-0.25 unless the agent had already completed all necessary work

Provide a justification (1-2 sentences) covering the overall process quality. Mention the strongest and weakest dimensions.

OUTPUT FORMAT - YOU MUST OUTPUT ONLY VALID JSON, NO OTHER TEXT:
{
  "process_quality_score": <float between 0.0 and 1.0>,
  "justification": "<1-2 sentences: overall quality + strongest/weakest dimension>"
}

DO NOT include markdown code blocks. DO NOT include explanatory text before or after the JSON. Output ONLY the JSON object."""
    }
}


def route_evaluation_axes(step: Dict, trajectory_context: Dict = None) -> List[str]:
    """
    Route to 2-axis system.
    
    Axes:
    - Correctness: Only when code is present
    - Process Quality: Always (covers reasoning + action + efficiency)
    """
    axes = []
    
    # Process quality: always evaluate (covers reasoning, action, efficiency)
    axes.append('process_quality')
    
    # Correctness: only when code is present
    tool_name = step.get('tool_name', '')
    action_type = step.get('action_type', '')
    
    if (tool_name in CODE_GENERATING_TOOLS or
        action_type in CODE_GENERATING_ACTIONS or
        step.get('code') or
        step.get('final_submission')):
        axes.append('correctness')
    
    assert all(ax in VALID_AXES for ax in axes), f"Unknown axis in: {axes}"
    return axes


def get_axis_weights(axes: List[str], step: Dict, trajectory_context: Dict = None) -> Dict[str, float]:
    """
    Assign weights for 2-axis system.
    
    Adjustments:
    - Final submission: Increase correctness weight (2.0×)
    - Error recovery/forced final/repeated: Increase process_quality weight (1.5×)
    """
    weights = {axis: 1.0 for axis in axes}
    
    tool_name = step.get('tool_name', '')
    step_reward = float(step.get('step_reward', 0.0))
    is_recovery = step_reward < -0.1
    is_forced_final = step.get('is_forced_final', False)
    consecutive = (trajectory_context or {}).get('consecutive_same_tool', 0)
    is_repeated = consecutive >= 2
    
    adjustments = {
        'correctness': 2.0 if tool_name == 'submit_solution' else 1.0,
        'process_quality': 1.5 if (is_recovery or is_forced_final or is_repeated) else 1.0,
    }
    
    for axis in axes:
        weights[axis] *= adjustments.get(axis, 1.0)
    
    total = sum(weights.values())
    return {k: round(v / total, 4) for k, v in weights.items()}


def needs_human_review(judge_score: float, mc_label: float) -> bool:
    """
    Flag trajectories where judge is optimistic but MC is pessimistic.
    
    This pattern indicates subtle bugs that the judge missed.
    
    Args:
        judge_score: Judge overall score (0.0-1.0)
        mc_label: MC label soft (0.0-1.0)
    
    Returns:
        True if this trajectory should be reviewed by humans
    """
    # Judge optimistic, MC pessimistic = subtle bug signature
    return judge_score > 0.4 and mc_label < 0.15


def aggregate_axis_scores(
    axis_scores: Dict[str, float], 
    weights: Dict[str, float],
    min_coverage: float = 0.6
) -> Dict:
    """
    Aggregate scores from multiple evaluation axes with reliability tracking.
    
    If an axis was routed but failed to return a score (API error, parse failure),
    we mark the aggregate as unreliable rather than silently injecting 0.5.
    
    Args:
        axis_scores: Dict mapping axis name to score (0.0-1.0)
        weights: Dict mapping axis name to weight (sums to 1.0)
        min_coverage: Minimum weight coverage to consider reliable (default 0.6)
    
    Returns:
        Dict with:
        - judge_overall_score: Weighted average (or None if unreliable)
        - coverage: Fraction of expected weight that was scored
        - reliable: Whether aggregate is trustworthy
    """
    # Only aggregate axes that were both routed AND scored
    scored_axes = {ax: s for ax, s in axis_scores.items() if ax in weights}
    scored_weight = sum(weights[ax] for ax in scored_axes)
    
    # Reject aggregate if coverage too low
    if scored_weight < min_coverage:
        return {
            "judge_overall_score": None,
            "coverage": round(scored_weight, 4),
            "reliable": False
        }
    
    # Renormalize weights over available axes only
    norm_weights = {ax: weights[ax] / scored_weight for ax in scored_axes}
    score = sum(scored_axes[ax] * norm_weights[ax] for ax in scored_axes)
    
    return {
        "judge_overall_score": round(score, 4),
        "coverage": round(scored_weight, 4),
        "reliable": True
    }


def compute_hybrid_label(step: Dict, judge_result: Dict, alpha: float = 0.4) -> Dict:
    """
    Blend MC labels with judge scores using specified alpha.
    
    Formula: combined_label = alpha × judge + (1-alpha) × MC
    
    Args:
        step: Step dict with MC labels
        judge_result: Result from aggregate_axis_scores()
        alpha: Weight for judge score (0.0 = MC only, 1.0 = judge only)
    
    Returns:
        Dict with multiple label variants for different alpha values
    """
    # Extract MC labels (support both field names)
    mc_label = step.get('mc_label_soft') if step.get('mc_label_soft') is not None else step.get('mc_advantage_normalized')
    mc_reliable = mc_label is not None and not step.get('mc_zero_data', False)
    
    # Extract judge labels
    judge_score = judge_result.get('judge_overall_score', None)
    judge_reliable = judge_result.get('reliable', False)
    
    # Generate labels for multiple alpha values
    alphas = [0.0, 0.2, 0.4, 0.6, 0.8]
    label_variants = {}
    
    for a in alphas:
        variant_name = f"alpha_{a}" if a not in [0.0, 0.8] else ("mc_only" if a == 0.0 else "process_heavy")
        
        if mc_reliable and judge_reliable:
            # Combined label
            label_variants[variant_name] = round(a * judge_score + (1 - a) * mc_label, 4)
        elif mc_reliable:
            # MC only (fallback)
            label_variants[variant_name] = mc_label
        elif judge_reliable:
            # Judge only (rare case)
            label_variants[variant_name] = judge_score
        else:
            # No reliable label
            label_variants[variant_name] = None
    
    # Determine primary label source
    if mc_reliable and judge_reliable:
        label_source = "hybrid"
        include_in_training = True
    elif mc_reliable:
        label_source = "mc_only"
        include_in_training = True
    elif judge_reliable:
        label_source = "judge_only"
        include_in_training = True
    else:
        label_source = "excluded"
        include_in_training = False
    
    # Consensus check
    consensus = None
    if mc_reliable and judge_reliable:
        # MC advantage is centered at 0; judge score is on 0-1 scale centered at 0.5
        mc_positive = mc_label > 0
        judge_positive = judge_score > 0.5
        consensus = (mc_positive == judge_positive)
    
    return {
        "prm_training_label": label_variants.get(f"alpha_{alpha}"),  # Default alpha
        "label_variants": label_variants,  # All alpha variants
        "label_source": label_source,
        "consensus": consensus,
        "include_in_training": include_in_training,
        "mc_label_original": mc_label,
        "judge_score_original": judge_score
    }


def needs_human_review(judge_score: float, mc_label: float) -> bool:
    """
    Flag trajectories where judge is optimistic but MC is pessimistic.
    
    This pattern indicates subtle bugs that the judge missed.
    
    Args:
        judge_score: Judge overall score (0.0-1.0)
        mc_label: MC label soft (0.0-1.0)
    
    Returns:
        True if this trajectory should be reviewed by humans
    """
    # Judge optimistic, MC pessimistic = subtle bug signature
    return judge_score > 0.4 and mc_label < 0.15


# Export for use in add_llm_judge.py
__all__ = [
    'EVALUATION_AXES',
    'VALID_AXES',
    'route_evaluation_axes',
    'get_axis_weights',
    'needs_human_review',
    'aggregate_axis_scores',
    'compute_hybrid_label',
]
