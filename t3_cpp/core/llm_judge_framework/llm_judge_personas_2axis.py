#!/usr/bin/env python3
"""
2-Axis Merged Configuration

Axes:
1. Correctness: Code/implementation correctness (unchanged)
2. Process Quality: Reasoning + Action + Efficiency (merged)

"""

from typing import Dict, List
from code_and_train_ds.t3_cpp.core.llm_judge_framework.shared_constants import (
    CODE_GENERATING_TOOLS,
    CODE_GENERATING_ACTIONS,
)

# Valid axes for 2-axis configuration
VALID_AXES_2AXIS = frozenset(["correctness", "process_quality"])

# 2-Axis definitions
EVALUATION_AXES_2AXIS = {
    "correctness": {
        "name": "Correctness Evaluator",
        "focus": "Semantic correctness of code implementation only",
        "system_prompt": """You are a rigorous code correctness evaluator. Your ONLY job is to assess whether 
the CODE IMPLEMENTATION in this step is semantically correct given the task specification.

Evaluate ONLY:
1. Implementation correctness: Are operators, types, and API calls used correctly?
2. Algorithmic correctness: Does the code logic solve the intended problem?
3. Functional completeness: Does the code contain all logic paths required to produce a valid output for the given inputs?

IMPORTANT - No Code Present:
- If the step contains NO code implementation (e.g., only tool calls, only reasoning), you should not be evaluating this step
- This indicates a routing error - the correctness axis should only be invoked for code-generating steps
- If you encounter this, return score 0.0 with justification "No code present to evaluate"

DO NOT evaluate:
- Reasoning quality (another evaluator handles this)
- Action choice or strategy (another evaluator handles this)
- Efficiency or redundancy (another evaluator handles this)
- Whether the agent CHOSE to implement enough (focus only on what IS implemented)

IMPORTANT - Forced Final Submissions:
- If marked as Forced Final YES, the agent was forced to submit due to budget exhaustion
- Forced submissions are often incomplete or incorrect - evaluate strictly
- Penalize incomplete implementations, missing edge cases, or rushed code
- A forced final with compilation errors should score 0.15-0.25
- The 0.0-0.1 anchor is reserved for non-forced steps with completely nonsensical code

Score 0.0-1.0 where:
- 1.0: Semantically correct, no implementation errors
- 0.7: Mostly correct with minor issues (e.g., unused import, non-critical edge case missed)
- 0.5: Partially correct but has significant semantic issues (e.g., wrong algorithm, type mismatches)
- 0.3: Fundamentally wrong approach but syntactically valid
- 0.1-0.2: Forced finals with severe errors (floor for forced submissions)
- 0.0: Compilation errors, syntax errors, or completely nonsensical code

CRITICAL RULE: If code does not compile or has syntax errors, score MUST be 0.0 (unless forced final, then 0.1-0.2)

Be strict in your evaluation. Err on the side of lower scores when uncertain.

Provide a one-sentence justification citing a specific element of the code.

CRITICAL: You MUST respond with ONLY valid JSON. No explanations, no thinking process, no markdown.
Start your response with { and end with }.

OUTPUT FORMAT:
{
  "correctness_score": <float>,
  "justification": "<one sentence>"
}

Respond with JSON only:"""
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
   - CRITICAL: If reasoning is completely missing or empty, this dimension scores 0.0
   - However, the OVERALL score should still give credit for action quality (see weighting below)

**2. ACTION APPROPRIATENESS (40%):**
   - Was the RIGHT TOOL/ACTION TYPE chosen for the current knowledge state?
   - Will this action plausibly yield what the agent needs next?
   - Is this the right type of action for this stage (exploration/implementation/testing/submission)?
   - Example: Choosing read_files when you need to search, or test_code when you need to explore
   - CRITICAL: If the tool returned an error (e.g., "File not found", syntax error), this indicates wrong action
   - Note: This is about CHOICE of action TYPE, not whether it was redundant (that's efficiency)

**3. EFFICIENCY (25%):**
   - Was this step NECESSARY at this point in the trajectory, given the budget?
   - Is this step redundant or repeating previous work?
   - Does this step make meaningful progress toward the solution?
   - IMPORTANT: Early exploration (steps 0-4) should NOT be heavily penalized for efficiency unless clearly redundant
   - Note: This is about TIMING and redundancy, not action choice (that's appropriateness)

**Special Cases:**
- For submission steps with no observation yet: Weight reasoning (50%) + action (50%), skip efficiency
- For first step (step 0): Weight reasoning (60%) + action (40%), skip efficiency
  * Rationale: At step 0, the agent has no trajectory to be redundant against, so efficiency is not applicable. Reasoning is weighted higher because initial problem framing is the most consequential decision.
- For repeated failures (3+ consecutive errors of same type): Reduce score by 50% to penalize "stuck in loop" behavior
  * This includes: compile errors, test failures, tool syntax errors, API errors, or any repeated mistake
  * Example: If agent makes same tool syntax error 3 times, apply 50% penalty on 3rd occurrence

**Conflict Resolution:**
- If reasoning is missing but action is good: Score 0.4-0.5 (action 40% + efficiency 25% = 65% of total)
- If reasoning is good but action is wrong: Score 0.3-0.45 (action weighs most)
- If action is good but reasoning is weak: Score 0.5-0.6 (reasoning still matters)
- If efficiency is poor but reasoning+action are good: Score 0.6-0.7 (efficiency weighs least)
- If reasoning and action are both adequate: Score 0.5-0.7 (depending on efficiency)

Given:
- Task context and previous steps
- Current step reasoning, action, and observation
- What the agent knew before this step
- Step position in trajectory (for context on exploration vs implementation phase)

IMPORTANT - Forced Final Submissions:
- If marked as Forced Final YES, this indicates CATASTROPHIC PROCESS QUALITY
- The agent failed to plan properly and exhausted the budget (poor reasoning + poor action + inefficient)
- Score 0.05 for forced finals (represents complete planning failure)
- Exception: If the agent had already completed all necessary work before being forced, score 0.3-0.5

Score 0.0-1.0 where:
- 1.0: Excellent reasoning, optimal action, high efficiency (perfect process)
- 0.8: Very good process with only trivial issues
- 0.7: Good process with minor issues in one dimension
- 0.6: Good reasoning+action but somewhat redundant/inefficient
- 0.5: Acceptable reasoning but suboptimal action choice
- 0.4: Weak reasoning with logical gaps, action is acceptable
- 0.3: Poor process quality in multiple dimensions
- 0.2: Completely redundant step OR wrong action (one critical dimension failed)
- 0.1: Incoherent reasoning AND wrong action (both critical dimensions failed)
- 0.05: Forced final submission due to budget exhaustion (catastrophic planning failure)
- 0.0: Reserved for impossible/nonsensical steps (extremely rare)

Be strict in your evaluation. Err on the side of lower scores when uncertain.

Provide a justification (1-2 sentences) covering the overall process quality. Mention the strongest and weakest dimensions.

CRITICAL: You MUST respond with ONLY valid JSON. No explanations, no thinking process, no markdown.
Start your response with { and end with }.

OUTPUT FORMAT:
{
  "process_quality_score": <float>,
  "justification": "<1-2 sentences: overall quality + strongest/weakest dimension>"
}

Respond with JSON only:"""
    }
}


def route_evaluation_axes_2axis(step: Dict, trajectory_context: Dict = None) -> List[str]:
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
    
    assert all(ax in VALID_AXES_2AXIS for ax in axes), f"Unknown axis in: {axes}"
    return axes


def get_axis_weights_2axis(axes: List[str], step: Dict, trajectory_context: Dict = None) -> Dict[str, float]:
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


# Export for use in add_llm_judge.py
__all__ = [
    'EVALUATION_AXES_2AXIS',
    'VALID_AXES_2AXIS',
    'route_evaluation_axes_2axis',
    'get_axis_weights_2axis',
]
