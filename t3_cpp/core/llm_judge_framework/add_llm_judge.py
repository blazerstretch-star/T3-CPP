#!/usr/bin/env python3
"""
LLM Judge Integration Script for t3_cpp

Reads MC-labeled trajectories and adds LLM judge evaluations.
Can run as Pass 3 after mc_prm_labeler_ml_fixed.py or standalone.

Usage:
    # After MC labeling
    python add_llm_judge.py \\
        --input Mcts_group_labeled_ds/labeled_steps_compact.jsonl \\
        --output Mcts_group_labeled_ds/labeled_steps_with_judge.jsonl \\
        --judge_model gpt-4o-mini \\
        --api_key $OPENAI_API_KEY

    # Or process full directory structure
    python add_llm_judge.py \\
        --input_dir Mcts_group_labeled_ds \\
        --output_dir Mcts_group_labeled_ds_judged \\
        --judge_model claude-3-5-haiku \\
        --api_key $ANTHROPIC_API_KEY
"""

import argparse
import asyncio
import json
import logging
import sys
import time
from collections import defaultdict
from pathlib import Path
from typing import Dict, List

from tqdm import tqdm

from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_evaluator import (
    LLMClient,
    APIConfig,
    JudgeCache,
    evaluate_step,
    compute_cache_key,
)
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_calibrated import (
    EVALUATION_AXES,
    route_evaluation_axes,
    get_axis_weights,
    aggregate_axis_scores,
    compute_hybrid_label,
)
# from work_stealing_scheduler import process_with_work_stealing  # Disabled due to indentation issues

logger = logging.getLogger("add_llm_judge")
# FAILURE_TRACKING_PATCHED
# BUGFIX #7: Make failure_stats async-safe with lock
failure_stats = {
    "api_errors": 0,
    "json_parse_errors": 0,
    "axis_evaluation_failures": 0,
    "cache_errors": 0,
    "trajectory_failures": 0,
    "step_failures": 0,
}
failure_stats_lock = asyncio.Lock()

async def log_failure_async(failure_type: str, details: str, trajectory_id: str = None, step_pos: int = None):
    """Log failure with context (async-safe)"""
    async with failure_stats_lock:
        failure_stats[failure_type] = failure_stats.get(failure_type, 0) + 1
    context = f"[Traj: {trajectory_id}, Step: {step_pos}]" if trajectory_id else ""
    print(f"\n⚠️  FAILURE {context}: {failure_type}")
    print(f"   Details: {details}")
    logger.error(f"{failure_type} {context}: {details}")

def log_failure(failure_type: str, details: str, trajectory_id: str = None, step_pos: int = None):
    """Log failure with context (sync wrapper for backward compatibility)"""
    # Try to use async version if in async context, otherwise update directly
    try:
        loop = asyncio.get_running_loop()
        # We're in async context but this is sync function - just update without lock (best effort)
        failure_stats[failure_type] = failure_stats.get(failure_type, 0) + 1
    except RuntimeError:
        # Not in async context - safe to update directly
        failure_stats[failure_type] = failure_stats.get(failure_type, 0) + 1
    
    context = f"[Traj: {trajectory_id}, Step: {step_pos}]" if trajectory_id else ""
    print(f"\n⚠️  FAILURE {context}: {failure_type}")
    print(f"   Details: {details}")
    logger.error(f"{failure_type} {context}: {details}")

def print_failure_summary():
    """Print summary of all failures"""
    print("\n" + "=" * 60)
    print("  FAILURE SUMMARY")
    print("=" * 60)
    total_failures = sum(failure_stats.values())
    if total_failures == 0:
        print("✓ No failures detected")
    else:
        print(f"Total failures: {total_failures}")
        for failure_type, count in sorted(failure_stats.items()):
            if count > 0:
                print(f"  {failure_type}: {count}")
    print("=" * 60 + "\n")



# ═══════════════════════════════════════════════════════════════════════════
# TRAJECTORY LOADING
# ═══════════════════════════════════════════════════════════════════════════

def load_trajectories_from_jsonl(jsonl_path: Path) -> Dict[str, tuple]:
    """
    Load trajectories from compact JSONL file.
    
    Returns:
        Dict mapping trajectory_id to (steps, rel_path) tuple
        BUGFIX #2: Normalized to match load_trajectories_from_directory shape
    """
    trajectories = defaultdict(list)
    
    with open(jsonl_path) as f:
        for line in f:
            if not line.strip():
                continue
            step = json.loads(line)
            traj_id = step.get("trajectory_id")
            if traj_id:
                trajectories[traj_id].append(step)
    
    # Sort steps by position and wrap in tuple with None rel_path
    result = {}
    for traj_id, steps in trajectories.items():
        sorted_steps = sorted(steps, key=lambda s: s.get("step_position", 0))
        result[traj_id] = (sorted_steps, None)  # None rel_path for single-file mode
    
    return result


def load_trajectories_from_directory(input_dir: Path) -> Dict[str, tuple]:
    """
    Load trajectories from directory structure (per-trajectory JSONL files).
    
    Returns:
        Dict mapping trajectory_id to (steps, relative_path) tuple
    """
    trajectories = {}
    
    jsonl_files = list(input_dir.rglob("*_labeled.jsonl"))
    
    for jsonl_file in tqdm(jsonl_files, desc="Loading trajectories"):
        steps = []
        with open(jsonl_file) as f:
            for line in f:
                if not line.strip():
                    continue
                step = json.loads(line)
                steps.append(step)
        
        if steps:
            traj_id = steps[0].get("trajectory_id")
            if traj_id:
                # Store relative path from input_dir
                rel_path = jsonl_file.relative_to(input_dir)
                trajectories[traj_id] = (
                    sorted(steps, key=lambda s: s.get("step_position", 0)),
                    rel_path
                )
    
    return trajectories


# ═══════════════════════════════════════════════════════════════════════════
# BATCH PROCESSING
# ═══════════════════════════════════════════════════════════════════════════

async def process_trajectories_batch(
    trajectories: Dict[str, tuple],  # BUGFIX #2: Normalized signature
    client: LLMClient,
    cache: JudgeCache,
    output_dir: Path,
    input_dir: Path,
    max_concurrent: int = 10,
    store_raw_responses: bool = False,
    step_batch_size: int = 5
) -> Dict[str, List[Dict]]:
    """
    Process trajectories with round-robin distribution across API keys.
    
    OPTIMIZED: Parallel axes + parallel steps within trajectory.
    
    Args:
        trajectories: Dict mapping trajectory_id to (steps, rel_path) tuple
        client: LLM API client with multiple API keys
        cache: Judge cache
        output_dir: Output directory root
        input_dir: Input directory root (for structure mirroring)
        max_concurrent: Number of API keys to use in parallel
        store_raw_responses: Store raw LLM responses
        step_batch_size: Number of steps to process in parallel (default: 5)
    
    Returns:
        Dict mapping trajectory_id to enriched steps
    """
    write_lock = asyncio.Lock()
    progress_lock = asyncio.Lock()
    
    # Progress tracking
    total_trajectories = len(trajectories)
    completed_trajectories = 0
    start_time = time.time()
    
    # Ensure output directory exists
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Distribute trajectories across API keys (round-robin)
    num_keys = len(client.api_keys)
    key_assignments = [[] for _ in range(num_keys)]
    
    for idx, (traj_id, (steps, rel_path)) in enumerate(trajectories.items()):
        key_idx = idx % num_keys
        key_assignments[key_idx].append((traj_id, steps, rel_path))
    
    logger.info(f"Distributed {len(trajectories)} trajectories across {num_keys} API keys")
    logger.info(f"OPTIMIZATION: Parallel axes + parallel steps (batch_size={step_batch_size})")
    for idx, assigned in enumerate(key_assignments[:10]):  # Show first 10
        logger.info(f"  API key {idx}: {len(assigned)} trajectories")
    if num_keys > 10:
        logger.info(f"  ... and {num_keys - 10} more keys")
    
    async def process_with_key(key_idx: int, assigned_trajs: List[tuple]):
        """Process assigned trajectories sequentially with one API key."""
        nonlocal completed_trajectories
        api_key = client.api_keys[key_idx]
        
        # Create dedicated log file for this worker in output_dir/logs/
        log_dir = output_dir / "logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        worker_log_path = log_dir / f"worker_{key_idx:03d}.log"
        worker_logger = logging.getLogger(f"worker_{key_idx}")
        worker_handler = logging.FileHandler(worker_log_path)
        worker_handler.setFormatter(logging.Formatter('%(asctime)s [%(levelname)s] %(message)s'))
        worker_logger.addHandler(worker_handler)
        worker_logger.setLevel(logging.INFO)
        
        worker_logger.info(f"Worker {key_idx} started with {len(assigned_trajs)} trajectories")
        
        # Create dedicated client for this key
        key_config = APIConfig(
            provider=client.config.provider,
            model=client.config.model,
            api_key=api_key,
            base_url=client.config.base_url,
            temperature=client.config.temperature,
            max_tokens=client.config.max_tokens,
            timeout=client.config.timeout,
        )
        
        key_cache = cache  # Share cache across all keys
        results = {}
        
        async with LLMClient(key_config, api_keys=[api_key]) as key_client:
            for traj_idx, (traj_id, steps, rel_path) in enumerate(assigned_trajs):
                try:
                    worker_logger.info(f"Starting trajectory {traj_idx+1}/{len(assigned_trajs)}: {traj_id}")
                    enriched_steps = []
                    
                    # Use the original relative path from input
                    output_path = output_dir / rel_path
                    output_path.parent.mkdir(parents=True, exist_ok=True)
                    
                    # OPTIMIZATION: Process steps in batches (parallel steps + parallel axes)
                    for batch_start in range(0, len(steps), step_batch_size):
                        batch_end = min(batch_start + step_batch_size, len(steps))
                        worker_logger.info(f"  Processing steps {batch_start+1}-{batch_end}/{len(steps)}")
                        
                        # BUGFIX #1: Snapshot enriched_steps before batch to prevent race condition
                        context_snapshot = list(enriched_steps)
                        
                        # Process batch of steps in parallel
                        async def process_single_step(step_idx: int, step: Dict):
                            """Process a single step with parallel axis evaluation."""
                            axes = route_evaluation_axes(step)
                            weights = get_axis_weights(axes, step)
                            
                            # OPTIMIZATION 1: Evaluate all axes in parallel with retry
                            axis_scores = {}
                            axis_failures = {}
                            
                            async def evaluate_axis_with_retry(axis: str, max_retries: int = 2) -> float:
                                """Wrapper to retry axis evaluation on transient failures."""
                                import aiohttp
                                last_exception = None
                                for attempt in range(max_retries):
                                    try:
                                        # BUGFIX #1: Use context_snapshot instead of enriched_steps
                                        result = await evaluate_step(step, axis, key_client, key_cache, context_snapshot)
                                        if result is not None:
                                            return result
                                        # None means parse/validation failure - don't retry
                                        return None
                                    except (asyncio.TimeoutError, ConnectionError) as e:
                                        # Transient failures - retry
                                        last_exception = e
                                        if attempt < max_retries - 1:
                                            wait_time = 2 ** attempt
                                            worker_logger.warning(f"    Axis {axis} attempt {attempt+1} failed: {type(e).__name__}, retrying in {wait_time}s")
                                            await asyncio.sleep(wait_time)
                                        continue
                                    except aiohttp.ClientResponseError as e:
                                        # BUGFIX #15: Retry 429/503/502 errors
                                        if e.status in (429, 503, 502):
                                            last_exception = e
                                            if attempt < max_retries - 1:
                                                wait_time = 2 ** attempt
                                                worker_logger.warning(f"    Axis {axis} HTTP {e.status} attempt {attempt+1}, retrying in {wait_time}s")
                                                await asyncio.sleep(wait_time)
                                            continue
                                        else:
                                            # Non-retryable HTTP error
                                            worker_logger.error(f"    Axis {axis} HTTP {e.status}: {str(e)}")
                                            raise
                                    except Exception as e:
                                        # Non-transient failures - don't retry
                                        worker_logger.error(f"    Axis {axis} failed: {type(e).__name__}: {str(e)}")
                                        raise
                                
                                # All retries exhausted
                                if last_exception:
                                    raise last_exception
                                return None
                            
                            # Create tasks for all axes
                            axis_tasks = {
                                axis: evaluate_axis_with_retry(axis)
                                for axis in axes
                            }
                            
                            # Execute in parallel (rate limiting happens inside evaluate_step)
                            axis_results = await asyncio.gather(
                                *axis_tasks.values(),
                                return_exceptions=True
                            )
                            
                            # Process results
                            for axis, result in zip(axis_tasks.keys(), axis_results):
                                if isinstance(result, Exception):
                                    worker_logger.error(f"    Step {step_idx+1} Axis {axis} failed: {type(result).__name__}")
                                    axis_failures[axis] = f"{type(result).__name__}: {str(result)[:100]}"
                                    log_failure("axis_evaluation_failures", 
                                              f"Axis {axis}: {type(result).__name__}: {str(result)[:100]}",
                                              step.get("trajectory_id"), step_idx)
                                elif result is not None:
                                    axis_scores[axis] = result
                                else:
                                    worker_logger.warning(f"    Step {step_idx+1} Axis {axis}: returned None")
                                    axis_failures[axis] = "returned_none"
                            
                            # Aggregate scores
                            judge_result = aggregate_axis_scores(axis_scores, weights)
                            hybrid_label = compute_hybrid_label(step, judge_result)
                            
                            # Build enriched step
                            enriched_step = {**step}
                            enriched_step['judge_axes_used'] = axes
                            enriched_step['judge_axis_weights'] = weights
                            enriched_step['judge_axis_scores'] = axis_scores
                            enriched_step['judge_axis_failures'] = axis_failures
                            enriched_step['judge_axis_justifications'] = {}
                            
                            if store_raw_responses:
                                enriched_step['judge_axis_raw_responses'] = {}
                            
                            # Extract justifications from cache (BUGFIX #1: use context_snapshot)
                            for axis in axes:
                                if axis in axis_scores:
                                    cache_key = compute_cache_key(step, axis, key_client.config.model, context_snapshot)
                                    cached = key_cache.get(cache_key)
                                    if cached:
                                        if 'justification' in cached:
                                            enriched_step['judge_axis_justifications'][axis] = cached['justification']
                                        if store_raw_responses:
                                            with key_cache._lock:
                                                cursor = key_cache.conn.execute(
                                                    "SELECT raw_response FROM judge_cache WHERE cache_key = ?",
                                                    (cache_key,)
                                                )
                                                row = cursor.fetchone()
                                                if row and row[0]:
                                                    enriched_step['judge_axis_raw_responses'][axis] = row[0]
                            
                            enriched_step.update(judge_result)
                            enriched_step.update(hybrid_label)
                            
                            return enriched_step
                        
                        # OPTIMIZATION 2: Process batch of steps in parallel
                        batch_steps = steps[batch_start:batch_end]
                        step_tasks = [
                            process_single_step(batch_start + i, step)
                            for i, step in enumerate(batch_steps)
                        ]
                        
                        batch_results = await asyncio.gather(*step_tasks, return_exceptions=True)
                        
                        # BUGFIX #3: Use zip to maintain step ordering
                        for step, result in zip(batch_steps, batch_results):
                            if isinstance(result, Exception):
                                worker_logger.error(f"  Step {step.get('step_position', '?')+1} failed: {result}")
                                # BUGFIX #4: Mark failed steps explicitly for resume logic
                                enriched_steps.append({**step, 'judge_overall_score': None, 'judge_failed': True})
                            else:
                                enriched_steps.append(result)
                                worker_logger.info(f"  Step {result.get('step_position', '?')+1} completed: score={result.get('judge_overall_score', 'N/A')}")
                    
                    # Write entire trajectory after completion
                    if output_path and enriched_steps:
                        async with write_lock:
                            with open(output_path, 'w') as f:
                                for step in enriched_steps:
                                    f.write(json.dumps(step, default=str) + "\n")
                        
                        worker_logger.info(f"Completed trajectory {traj_id} ({len(enriched_steps)} steps) -> {output_path.name}")
                    
                    results[traj_id] = enriched_steps
                    
                    # Update progress
                    async with progress_lock:
                        completed_trajectories += 1
                        elapsed = time.time() - start_time
                        progress_pct = (completed_trajectories / total_trajectories) * 100
                        eta_seconds = (elapsed / completed_trajectories) * (total_trajectories - completed_trajectories) if completed_trajectories > 0 else 0
                        eta_minutes = int(eta_seconds // 60)
                        eta_hours = int(eta_minutes // 60)
                        eta_minutes = eta_minutes % 60
                        
                        # Progress bar
                        bar_length = 40
                        filled = int(bar_length * completed_trajectories / total_trajectories)
                        bar = '█' * filled + '░' * (bar_length - filled)
                        
                        print(f"\r[{bar}] {completed_trajectories}/{total_trajectories} ({progress_pct:.1f}%) | "
                              f"Elapsed: {int(elapsed//60)}m {int(elapsed%60)}s | "
                              f"ETA: {eta_hours}h {eta_minutes}m", end='', flush=True)
                    
                except Exception as e:
                    worker_logger.error(f"Failed trajectory {traj_id}: {type(e).__name__}: {str(e)}")
                    log_failure("trajectory_failures", f"{type(e).__name__}: {str(e)}", traj_id)
                    print(f"\n❌ Trajectory {traj_id} FAILED: {type(e).__name__}")
        
        worker_logger.info(f"Worker {key_idx} finished: {len(results)} trajectories completed")
        
        # BUGFIX #8: Ensure handler cleanup in finally block
        try:
            worker_handler.close()
            worker_logger.removeHandler(worker_handler)
        except Exception as e:
            logger.warning(f"Worker {key_idx} handler cleanup failed: {e}")
        
        return results
    
    # Process all key assignments in parallel
    tasks = [
        process_with_key(idx, assigned)
        for idx, assigned in enumerate(key_assignments)
        if assigned  # Only create task if key has trajectories
    ]
    
    logger.info(f"Starting {len(tasks)} parallel workers (one per API key)")
    
    all_results = {}
    completed = 0
    
    for coro in asyncio.as_completed(tasks):
        try:
            key_results = await coro
            all_results.update(key_results)
            completed += 1
            logger.info(f"Worker {completed}/{len(tasks)} finished, total trajectories: {len(all_results)}")
        except Exception as e:
            logger.error(f"API key worker failed: {type(e).__name__}: {str(e)}")
    
    return all_results


def load_already_processed(output_path: Path) -> set:
    """
    Load trajectory IDs already in output file for resume capability.
    
    Args:
        output_path: Path to output JSONL file
    
    Returns:
        Set of trajectory IDs that have been processed
    """
    if not output_path.exists():
        return set()
    
    processed = set()
    with open(output_path) as f:
        for line in f:
            if line.strip():
                try:
                    step = json.loads(line)
                    # BUGFIX #4: Exclude failed steps from resume (judge_failed=True or score=None without explicit failure)
                    if step.get("judge_overall_score") is not None and not step.get("judge_failed", False):
                        processed.add(step.get("trajectory_id"))
                except json.JSONDecodeError:
                    continue
    
    return processed


# ═══════════════════════════════════════════════════════════════════════════
# OUTPUT WRITING
# ═══════════════════════════════════════════════════════════════════════════

def write_compact_jsonl(
    trajectories: Dict[str, List[Dict]],
    output_path: Path
):
    """Write all trajectories to a single compact JSONL file."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    
    with open(output_path, "w") as f:
        for traj_id in sorted(trajectories.keys()):
            for step in trajectories[traj_id]:
                f.write(json.dumps(step, default=str) + "\n")
    
    logger.info(f"Wrote {len(trajectories)} trajectories to {output_path}")


def write_directory_structure(
    trajectories: Dict[str, List[Dict]],
    output_dir: Path,
    input_dir: Path
):
    """
    Write trajectories preserving directory structure.
    
    Mirrors the input directory structure in output_dir.
    """
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Group by relative path
    by_path = defaultdict(list)
    for traj_id, steps in trajectories.items():
        if not steps:
            continue
        # Reconstruct relative path from trajectory metadata
        provider = steps[0].get("trajectory_id", "").split("__")[0]
        model = steps[0].get("model", "unknown")
        task_id = steps[0].get("task_id", "unknown")
        beam_id = steps[0].get("beam_id", 0)
        
        rel_path = Path(provider) / model / task_id / f"beam_{beam_id}_log_trj_labeled.jsonl"
        by_path[rel_path].append((traj_id, steps))
    
    # Write files
    for rel_path, traj_list in tqdm(by_path.items(), desc="Writing files"):
        output_path = output_dir / rel_path
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, "w") as f:
            for traj_id, steps in traj_list:
                for step in steps:
                    f.write(json.dumps(step, default=str) + "\n")


# ═══════════════════════════════════════════════════════════════════════════
# STATISTICS
# ═══════════════════════════════════════════════════════════════════════════

def print_statistics(
    trajectories: Dict[str, List[Dict]],
    client: LLMClient,
    cache: JudgeCache
):
    """Print evaluation statistics."""
    total_steps = sum(len(steps) for steps in trajectories.values())
    
    # Axis usage
    axis_counts = defaultdict(int)
    for steps in trajectories.values():
        for step in steps:
            for axis in step.get("judge_axes_used", []):
                axis_counts[axis] += 1
    
    # Judge scores (only reliable ones)
    judge_scores = [
        step.get("judge_overall_score")
        for steps in trajectories.values()
        for step in steps
        if step.get("judge_overall_score") is not None
    ]
    
    reliable_count = sum(
        1 for steps in trajectories.values()
        for step in steps
        if step.get("reliable", False)
    )
    
    coverage = len(judge_scores) / total_steps if total_steps > 0 else 0
    avg_judge_score = sum(judge_scores) / len(judge_scores) if judge_scores else 0.0
    
    # Training inclusion
    training_count = sum(
        1 for steps in trajectories.values()
        for step in steps
        if step.get("include_in_training", False)
    )
    
    # Label sources
    label_sources = defaultdict(int)
    for steps in trajectories.values():
        for step in steps:
            source = step.get("label_source", "unknown")
            label_sources[source] += 1
    
    print("\n" + "=" * 60)
    print("  LLM Judge Evaluation Statistics")
    print("=" * 60)
    print(f"\n  Trajectories evaluated : {len(trajectories):,}")
    print(f"  Total steps evaluated  : {total_steps:,}")
    print(f"  Avg judge score (reliable) : {avg_judge_score:.3f}")
    print(f"  Coverage (reliable steps)  : {coverage:.1%} ({reliable_count:,} / {total_steps:,})")
    print(f"  Training inclusion         : {training_count:,} / {total_steps:,} ({100*training_count/total_steps:.1f}%)" if total_steps > 0 else "  Training inclusion         : 0 / 0 (0.0%)")
    
    print(f"\n  Evaluation Axis Usage:")
    for axis, count in sorted(axis_counts.items()):
        print(f"    {axis:<30} {count:>6,} steps")
    
    print(f"\n  Label Sources:")
    for source, count in sorted(label_sources.items()):
        pct = 100 * count / total_steps if total_steps > 0 else 0
        print(f"    {source:<30} {count:>6,} ({pct:.1f}%)")
    
    print(f"\n  API Statistics:")
    print(f"    Input tokens           : {client.total_input_tokens:,}")
    print(f"    Output tokens          : {client.total_output_tokens:,}")
    print(f"    Estimated cost         : ${client.total_cost:.2f}")
    
    # Fallback model statistics
    if hasattr(client, 'fallback_used_count'):
        total_calls = client.primary_success_count + client.primary_failure_count
        fallback_rate = (client.fallback_used_count / total_calls * 100) if total_calls > 0 else 0
        print(f"\n  Fallback Model Statistics:")
        print(f"    Primary model successes: {client.primary_success_count:,}")
        print(f"    Primary model failures : {client.primary_failure_count:,}")
        print(f"    Fallback model used    : {client.fallback_used_count:,} ({fallback_rate:.1f}% of calls)")
    
    cache_stats = cache.stats()
    print(f"\n  Cache Statistics:")
    print(f"    Total cached entries   : {cache_stats['total']:,}")
    for axis, count in sorted(cache_stats.get('by_axis', {}).items()):
        print(f"      {axis:<28} {count:>6,}")
    
    print("=" * 60 + "\n")


# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════

async def main_async(args):
    """Async main function."""
    
    # BUGFIX #5: Load API keys once (removed duplicate loading)
    # Priority: .env file > --api_keys_file > --api_key > env var
    api_keys = []
    
    # Priority 1: Load from .env file (if exists)
    env_path = Path('.env')
    if env_path.exists():
        with open(env_path) as f:
            api_keys = [line.strip() for line in f if line.strip() and line.strip().startswith('nvapi-')]
        if api_keys:
            logger.info(f"Loaded {len(api_keys)} API keys from .env file")
    
    # Priority 2: Load from --api_keys_file (if provided) - OVERRIDES .env
    if args.api_keys_file:
        with open(args.api_keys_file) as f:
            api_keys = [line.strip() for line in f if line.strip()]
        logger.info(f"Loaded {len(api_keys)} API keys from {args.api_keys_file}")
    
    # Priority 3: Use single --api_key argument (if no file sources)
    if not api_keys and args.api_key:
        api_keys = [args.api_key]
        logger.info(f"Using single API key from --api_key argument")
    
    # Setup API client
    api_config = APIConfig(
        provider=args.provider,
        model=args.judge_model,
        api_key=api_keys[0] if api_keys else None,  # Fallback for config
        base_url=args.base_url,
        temperature=args.temperature,
        max_tokens=args.max_tokens,
        timeout=args.timeout,
        rpm_limit=args.rpm_limit,
        # Fallback model configuration
        fallback_model=None if args.disable_fallback else args.fallback_model,
        fallback_temperature=args.fallback_temperature,
        fallback_max_tokens=args.fallback_max_tokens,
    )
    
    # Setup cache
    cache_dir = Path(args.cache_dir)
    cache_dir.mkdir(parents=True, exist_ok=True)
    cache_filename = f"{args.judge_model.replace(':', '_').replace('/', '_')}.db"
    cache_path = cache_dir / cache_filename
    
    # Clear cache if requested
    if args.clear_cache and cache_path.exists():
        logger.info(f"Clearing cache: {cache_path}")
        cache_path.unlink()
    
    cache = JudgeCache(cache_path)
    
    logger.info(f"Using cache: {cache_path}")
    logger.info(f"Cache contains {cache.stats()['total']} entries")
    
    # Load trajectories
    if args.input:
        logger.info(f"Loading trajectories from {args.input}")
        trajectories = load_trajectories_from_jsonl(Path(args.input))
    elif args.input_dir:
        logger.info(f"Loading trajectories from {args.input_dir}")
        trajectories = load_trajectories_from_directory(Path(args.input_dir))
    else:
        logger.error("Must specify --input or --input_dir")
        sys.exit(1)
    
    logger.info(f"Loaded {len(trajectories)} trajectories")
    
    # Sample if requested
    if args.sample_rate < 1.0:
        import random
        random.seed(args.seed)
        sample_size = int(len(trajectories) * args.sample_rate)
        traj_ids = random.sample(list(trajectories.keys()), sample_size)
        trajectories = {tid: trajectories[tid] for tid in traj_ids}
        logger.info(f"Sampled {len(trajectories)} trajectories ({args.sample_rate:.1%})")
    
    # Resume from checkpoint if output exists
    if args.output and Path(args.output).exists() and not args.overwrite:
        already_done = load_already_processed(Path(args.output))
        trajectories = {tid: steps for tid, steps in trajectories.items() 
                        if tid not in already_done}
        logger.info(f"Resuming: {len(already_done)} done, {len(trajectories)} remaining")
    elif args.output_dir and Path(args.output_dir).exists() and not args.overwrite:
        # Resume for directory mode: check which trajectory files already exist
        already_done = set()
        for traj_id, (steps, rel_path) in list(trajectories.items()):
            output_path = Path(args.output_dir) / rel_path
            if output_path.exists():
                # Verify file has complete trajectory (all steps have judge scores)
                try:
                    with open(output_path) as f:
                        lines = [json.loads(line) for line in f if line.strip()]
                    if len(lines) == len(steps) and all(s.get('judge_overall_score') is not None or s.get('judge_axis_scores') for s in lines):
                        already_done.add(traj_id)
                except Exception as e:
                    logger.warning(f"Could not verify {output_path}: {e}, will reprocess")
        
        trajectories = {tid: data for tid, data in trajectories.items() if tid not in already_done}
        logger.info(f"Resuming (directory mode): {len(already_done)} done, {len(trajectories)} remaining")
    
    # Process trajectories
    async with LLMClient(api_config, api_keys=api_keys) as client:
        # Add progress tracking
        total_trajectories = len(trajectories)
        processed_count = 0
        start_time = time.time()
        
        logger.info(f"Starting evaluation of {total_trajectories} trajectories...")
        print(f"\n{'='*60}")
        print(f"Starting LLM Judge Evaluation")
        print(f"Total trajectories: {total_trajectories}")
        print(f"Started at: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"{'='*60}\n")
        
        # Determine output handling
        if args.output:
            # Single file output - use temp approach
            output_path = Path(args.output)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Write to temp file with incremental checkpointing
            temp_path = output_path.parent / f"_temp_{output_path.name}"
            
            # Simple version for single file output
            enriched = await process_trajectories_batch(
                trajectories, client, cache, temp_path.parent, Path(args.input_dir) if args.input_dir else Path('.'),
                args.max_concurrent, args.store_raw_responses, args.step_batch_size
            )
            
            # Consolidate to single file
            write_compact_jsonl(enriched, output_path)
            
        elif args.output_dir:
            # Directory output - use static assignment (work-stealing disabled)
            # BUGFIX #10: Force static assignment due to work_stealing indentation issues
            use_work_stealing = False  # Disabled
            
            logger.info("Using static round-robin assignment")
            enriched = await process_trajectories_batch(
                trajectories, client, cache, Path(args.output_dir), Path(args.input_dir),
                args.max_concurrent, args.store_raw_responses, args.step_batch_size
            )
        else:
            logger.error("Must specify --output or --output_dir")
            sys.exit(1)
        
        # Print statistics
        end_time = time.time()
        total_time = end_time - start_time
        hours = int(total_time // 3600)
        minutes = int((total_time % 3600) // 60)
        seconds = int(total_time % 60)
        
        print(f"\n{'='*60}")
        print(f"Evaluation Completed!")
        print(f"Completed at: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Total time: {hours:02d}:{minutes:02d}:{seconds:02d} ({total_time:.1f}s)")
        if len(enriched) > 0:
            avg_time = total_time / len(enriched)
            print(f"Avg time per trajectory: {avg_time:.1f}s")
        print(f"{'='*60}\n")
        
        # Only print detailed statistics if we have enriched data in memory
        if enriched:
            print_statistics(enriched, client, cache)
        else:
            # Work-stealing mode: trajectories written to disk, print basic stats
            print(f"\n{'='*60}")
            print(f"  API Statistics")
            print(f"{'='*60}")
            print(f"  Input tokens           : {client.total_input_tokens:,}")
            print(f"  Output tokens          : {client.total_output_tokens:,}")
            print(f"  Estimated cost         : ${client.total_cost:.2f}")
            
            cache_stats = cache.stats()
            print(f"\n  Cache Statistics:")
            print(f"    Total cached entries   : {cache_stats['total']:,}")
            for axis, count in sorted(cache_stats.get('by_axis', {}).items()):
                print(f"      {axis:<28} {count:>6,}")
            print(f"{'='*60}\n")
    
    cache.close()
    print_failure_summary()
    logger.info("Done!")


def main():
    parser = argparse.ArgumentParser(
        description="Add LLM judge evaluations to MC-labeled trajectories",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    
    # Input/output
    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument(
        "--input", type=Path,
        help="Input compact JSONL file (from MC labeler)"
    )
    input_group.add_argument(
        "--input_dir", type=Path,
        help="Input directory with per-trajectory JSONL files"
    )
    
    output_group = parser.add_mutually_exclusive_group(required=True)
    output_group.add_argument(
        "--output", type=Path,
        help="Output compact JSONL file"
    )
    output_group.add_argument(
        "--output_dir", type=Path,
        help="Output directory (mirrors input structure)"
    )
    
    # API configuration
    parser.add_argument(
        "--provider", type=str, default="openai",
        choices=["openai", "anthropic", "ollama", "nvidia"],
        help="LLM API provider"
    )
    parser.add_argument(
        "--judge_model", type=str, default="gpt-4o-mini",
        help="Model for LLM judge (gpt-4o-mini, claude-3-5-haiku, etc.)"
    )
    parser.add_argument(
        "--api_key", type=str,
        help="Single API key (or set OPENAI_API_KEY / ANTHROPIC_API_KEY / NVIDIA_API_KEY env var). Priority 3: used only if .env and --api_keys_file not available."
    )
    parser.add_argument(
        "--api_keys_file", type=Path,
        help="File with multiple API keys (one per line) for parallel processing. Priority 2: overrides .env file if provided."
    )
    parser.add_argument(
        "--base_url", type=str,
        help="Custom API base URL (for Ollama or proxies)"
    )
    
    # Fallback model configuration
    parser.add_argument(
        "--fallback_model", type=str, default="meta/llama-3.1-405b-instruct",
        help="Fallback model to use on timeout/API errors (default: meta/llama-3.1-405b-instruct)"
    )
    parser.add_argument(
        "--fallback_temperature", type=float, default=0.2,
        help="Temperature for fallback model (default: 0.2)"
    )
    parser.add_argument(
        "--fallback_max_tokens", type=int, default=1024,
        help="Max tokens for fallback model (default: 1024)"
    )
    parser.add_argument(
        "--disable_fallback", action="store_true",
        help="Disable fallback model (fail immediately on primary model errors)"
    )
    
    # API parameters
    parser.add_argument(
        "--temperature", type=float, default=0.0,
        help="LLM temperature (0.0 for deterministic)"
    )
    parser.add_argument(
        "--max_tokens", type=int, default=4096,
        help="Max tokens per API call (increased to capture full justifications)"
    )
    parser.add_argument(
        "--timeout", type=int, default=240,
        help="API timeout in seconds (base 240s, increases to 360s, 480s on retries)"
    )
    parser.add_argument(
        "--rpm_limit", type=int, default=35,
        help="Rate limit: requests per minute per API key (default: 35)"
    )
    
    # Performance
    parser.add_argument(
        "--max_concurrent", type=int, default=10,
        help="Max concurrent API calls"
    )
    parser.add_argument(
        "--step_batch_size", type=int, default=5,
        help="Number of steps to process in parallel within a trajectory (default: 5, use 3 if hitting rate limits)"
    )
    # BUGFIX #10: Fix work_stealing flag logic
    parser.add_argument(
        "--no_work_stealing", action="store_true",
        help="Disable work-stealing scheduler (use static round-robin assignment instead)"
    )
    parser.add_argument(
        "--cache_dir", type=Path, default=Path(".judge_cache"),
        help="Cache directory for LLM responses"
    )
    parser.add_argument(
        "--store_raw_responses", action="store_true",
        help="Store raw LLM responses in output JSONL (increases file size significantly)"
    )
    
    # Sampling
    parser.add_argument(
        "--sample_rate", type=float, default=1.0,
        help="Sample rate (0.0-1.0) for testing"
    )
    parser.add_argument(
        "--seed", type=int, default=42,
        help="Random seed for reproducible sampling"
    )
    
    # Resume capability
    parser.add_argument(
        "--overwrite", action="store_true",
        help="Overwrite existing output (default: resume from checkpoint)"
    )
    parser.add_argument(
        "--clear_cache", action="store_true",
        help="Clear the judge cache before starting evaluation"
    )
    
    # Logging
    parser.add_argument(
        "--verbose", action="store_true",
        help="Enable verbose logging"
    )
    
    args = parser.parse_args()
    
    # Setup logging
    logging.basicConfig(
        level=logging.INFO if args.verbose else logging.WARNING,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        stream=sys.stderr,
    )
    
    # Get API key - Priority: .env file > --api_keys_file > --api_key > env var
    # BUGFIX #5: Removed duplicate .env loading (now only in main_async)
    if not args.api_key and not args.api_keys_file:
        # Check if .env exists (will be loaded in main_async)
        env_path = Path('.env')
        if not env_path.exists():
            # Fallback to environment variable only if no .env file
            import os
            if args.provider == "openai":
                args.api_key = os.getenv("OPENAI_API_KEY")
            elif args.provider == "anthropic":
                args.api_key = os.getenv("ANTHROPIC_API_KEY")
            elif args.provider == "nvidia":
                args.api_key = os.getenv("NVIDIA_API_KEY")
            
            if args.api_key:
                logger.info(f"Loaded API key from environment variable")
        
        if not args.api_key and not env_path.exists() and args.provider not in ["ollama"]:
            parser.error(f"--api_key required for {args.provider} (or set env var or create .env file)")
    
    # Run async main
    asyncio.run(main_async(args))


if __name__ == "__main__":
    main()
