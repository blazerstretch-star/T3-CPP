#!/usr/bin/env python3
"""
Batch Step Evaluator - Evaluate multiple steps in a single LLM call

Reduces API calls by 3-5× while maintaining evaluation quality.
"""

import json
from typing import Dict, List, Optional
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_evaluator import LLMClient, JudgeCache


async def evaluate_step_batch(
    steps: List[Dict],
    axis: str,
    client: LLMClient,
    cache: JudgeCache,
    batch_size: int = 3
) -> List[Optional[float]]:
    """
    Evaluate multiple consecutive steps in a single LLM call.
    
    Args:
        steps: List of step dicts to evaluate
        axis: Evaluation axis (correctness, reasoning_coherence, etc.)
        client: LLM API client
        cache: Judge cache
        batch_size: Number of steps per API call (default: 3)
    
    Returns:
        List of scores (one per step), None for failed evaluations
    """
    from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_2axis import EVALUATION_AXES_2AXIS
    
    axis_config = EVALUATION_AXES_2AXIS.get(axis)
    if not axis_config:
        return [None] * len(steps)
    
    # Build batch prompt
    batch_prompt = f"""You are evaluating {len(steps)} consecutive steps from a code generation trajectory.

{axis_config['system_prompt']}

IMPORTANT: Evaluate each step independently and return a JSON array with {len(steps)} entries.

"""
    
    # Add each step
    for i, step in enumerate(steps):
        batch_prompt += f"""
--- STEP {i+1} ---
Position: {step.get('step_position', i)}
Action: {step.get('tool_name', 'unknown')}
Reasoning: {step.get('reasoning_normalized', step.get('reasoning', 'N/A'))[:500]}
Code: {step.get('code', 'N/A')[:500] if step.get('code') else 'N/A'}
Observation: {step.get('observation', 'N/A')[:300]}

"""
    
    # Request format
    score_field = f"{axis}_score"
    batch_prompt += f"""
OUTPUT FORMAT (valid JSON array only):
[
  {{"{score_field}": <float 0.0-1.0>, "justification": "<one sentence>"}},
  {{"{score_field}": <float 0.0-1.0>, "justification": "<one sentence>"}},
  {{"{score_field}": <float 0.0-1.0>, "justification": "<one sentence>"}}
]

DO NOT include markdown code blocks. Output ONLY the JSON array.
"""
    
    # Make API call
    try:
        response = await client.generate(batch_prompt)
        
        # Parse JSON array
        response_text = response.strip()
        if response_text.startswith("```"):
            # Remove markdown code blocks
            response_text = response_text.split("```")[1]
            if response_text.startswith("json"):
                response_text = response_text[4:]
        
        results = json.loads(response_text)
        
        if not isinstance(results, list) or len(results) != len(steps):
            return [None] * len(steps)
        
        # Extract scores
        scores = []
        for result in results:
            score = result.get(score_field)
            if isinstance(score, (int, float)) and 0.0 <= score <= 1.0:
                scores.append(float(score))
            else:
                scores.append(None)
        
        return scores
        
    except Exception as e:
        print(f"Batch evaluation failed: {e}")
        return [None] * len(steps)


async def evaluate_trajectory_batch(
    steps: List[Dict],
    axes: List[str],
    client: LLMClient,
    cache: JudgeCache,
    step_batch_size: int = 3
) -> List[Dict]:
    """
    Evaluate entire trajectory with batched step evaluation.
    
    Args:
        steps: All steps in trajectory
        axes: Evaluation axes to use
        client: LLM API client
        cache: Judge cache
        step_batch_size: Steps per API call (default: 3)
    
    Returns:
        List of enriched steps with scores
    """
    import asyncio
    from llm_judge_personas import aggregate_axis_scores, compute_hybrid_label
    from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_2axis import get_axis_weights_2axis
    
    enriched_steps = []
    
    # Process in batches
    for batch_start in range(0, len(steps), step_batch_size):
        batch_end = min(batch_start + step_batch_size, len(steps))
        batch_steps = steps[batch_start:batch_end]
        
        # Evaluate all axes for this batch in parallel
        axis_tasks = {
            axis: evaluate_step_batch(batch_steps, axis, client, cache, len(batch_steps))
            for axis in axes
        }
        
        axis_results = await asyncio.gather(*axis_tasks.values())
        axis_scores_batch = {
            axis: scores 
            for axis, scores in zip(axis_tasks.keys(), axis_results)
        }
        
        # Build enriched steps
        for i, step in enumerate(batch_steps):
            # Get scores for this step
            step_axis_scores = {
                axis: scores[i] 
                for axis, scores in axis_scores_batch.items()
                if scores[i] is not None
            }
            
            # Aggregate
            weights = get_axis_weights_2axis(axes, step)
            judge_result = aggregate_axis_scores(step_axis_scores, weights)
            hybrid_label = compute_hybrid_label(step, judge_result)
            
            enriched_step = {**step}
            enriched_step['judge_axes_used'] = axes
            enriched_step['judge_axis_scores'] = step_axis_scores
            enriched_step.update(judge_result)
            enriched_step.update(hybrid_label)
            
            enriched_steps.append(enriched_step)
    
    return enriched_steps
