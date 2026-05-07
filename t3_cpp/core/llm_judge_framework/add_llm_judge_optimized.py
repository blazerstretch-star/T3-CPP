#!/usr/bin/env python3
"""
OPTIMIZED LLM Judge Integration Script

Implements 3 levels of parallelization:
1. Parallel axes (2.4× speedup)
2. Parallel steps (9.6× speedup) 
3. Parallel trajectories (24× speedup)

Usage: Same as add_llm_judge.py but with --optimization_level flag
"""

import argparse
import asyncio
import json
import logging
import sys
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
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_calibrated import EVALUATION_AXES
from llm_judge_personas import (
    route_evaluation_axes,
    get_axis_weights,
    aggregate_axis_scores,
    compute_hybrid_label,
)

logger = logging.getLogger("add_llm_judge")


# Import original functions
from code_and_train_ds.t3_cpp.core.llm_judge_framework.add_llm_judge import (
    load_trajectories_from_jsonl,
    load_trajectories_from_directory,
    load_already_processed,
    write_compact_jsonl,
    write_directory_structure,
    print_statistics,
)


# ═══════════════════════════════════════════════════════════════════════════
# OPTIMIZED BATCH PROCESSING
# ═══════════════════════════════════════════════════════════════════════════

async def process_single_step_parallel_axes(
    step: Dict,
    step_idx: int,
    context_steps: List[Dict],
    key_client: LLMClient,
    key_cache: JudgeCache,
    worker_logger: logging.Logger,
    store_raw_responses: bool = False
) -> Dict:
    """Process a single step with parallel axis evaluation (Option 1)."""
    axes = route_evaluation_axes(step)
    weights = get_axis_weights(axes, step)
    
    # OPTIMIZATION 1: Evaluate all axes in parallel
    axis_scores = {}
    axis_failures = {}
    
    async def evaluate_axis_with_retry(axis: str, max_retries: int = 2) -> float:
        """Wrapper to retry axis evaluation on transient failures."""
        last_exception = None
        for attempt in range(max_retries):
            try:
                result = await evaluate_step(step, axis, key_client, key_cache, context_steps)
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
            worker_logger.error(f"    Axis {axis} failed: {type(result).__name__}: {str(result)}")
            axis_failures[axis] = f"{type(result).__name__}: {str(result)[:100]}"
        elif result is not None:
            axis_scores[axis] = result
            worker_logger.info(f"    Axis {axis}: {result:.3f}")
        else:
            worker_logger.warning(f"    Axis {axis}: returned None")
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
    
    # Extract justifications from cache
    for axis in axes:
        if axis in axis_scores:
            cache_key = compute_cache_key(step, axis, key_client.config.model, context_steps)
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


async def process_trajectory_parallel_steps(
    traj_id: str,
    steps: List[Dict],
    rel_path: Path,
    key_client: LLMClient,
    key_cache: JudgeCache,
    worker_logger: logging.Logger,
    output_dir: Path,
    write_lock: asyncio.Lock,
    store_raw_responses: bool = False,
    step_batch_size: int = 5
) -> List[Dict]:
    """Process a trajectory with parallel step evaluation (Option 2)."""
    worker_logger.info(f"Starting trajectory: {traj_id}")
    enriched_steps = []
    
    # OPTIMIZATION 2: Process steps in batches
    for batch_start in range(0, len(steps), step_batch_size):
        batch_end = min(batch_start + step_batch_size, len(steps))
        batch_steps = steps[batch_start:batch_end]
        
        worker_logger.info(f"  Processing steps {batch_start+1}-{batch_end}/{len(steps)}")
        
        # Create tasks for all steps in batch
        step_tasks = []
        for i, step in enumerate(batch_steps):
            actual_idx = batch_start + i
            context = enriched_steps  # All previously processed steps
            step_tasks.append(
                process_single_step_parallel_axes(
                    step, actual_idx, context, key_client, key_cache,
                    worker_logger, store_raw_responses
                )
            )
        
        # Execute batch in parallel
        batch_results = await asyncio.gather(*step_tasks, return_exceptions=True)
        
        # Process results
        for i, result in enumerate(batch_results):
            if isinstance(result, Exception):
                worker_logger.error(f"  Step {batch_start+i+1} failed: {result}")
                # Create dummy enriched step
                enriched_steps.append({**batch_steps[i], 'judge_overall_score': None})
            else:
                enriched_steps.append(result)
    
    # Write trajectory
    output_path = output_dir / rel_path
    output_path.parent.mkdir(parents=True, exist_ok=True)
    
    async with write_lock:
        with open(output_path, 'w') as f:
            for step in enriched_steps:
                f.write(json.dumps(step, default=str) + "\n")
    
    worker_logger.info(f"Completed trajectory {traj_id} ({len(enriched_steps)} steps)")
    return enriched_steps


async def process_trajectories_optimized(
    trajectories: Dict[str, tuple],
    client: LLMClient,
    cache: JudgeCache,
    output_dir: Path,
    input_dir: Path,
    optimization_level: int = 1,
    step_batch_size: int = 5,
    trajectory_batch_size: int = 10,
    store_raw_responses: bool = False
) -> Dict[str, List[Dict]]:
    """
    Process trajectories with configurable optimization level.
    
    Args:
        optimization_level: 1=parallel axes, 2=+parallel steps, 3=+parallel trajectories
        step_batch_size: Number of steps to process in parallel (level 2+)
        trajectory_batch_size: Number of trajectories to process in parallel (level 3)
    """
    write_lock = asyncio.Lock()
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Distribute trajectories across API keys
    num_keys = len(client.api_keys)
    key_assignments = [[] for _ in range(num_keys)]
    
    for idx, (traj_id, (steps, rel_path)) in enumerate(trajectories.items()):
        key_idx = idx % num_keys
        key_assignments[key_idx].append((traj_id, steps, rel_path))
    
    logger.info(f"Distributed {len(trajectories)} trajectories across {num_keys} API keys")
    logger.info(f"Optimization level: {optimization_level}")
    
    async def process_with_key(key_idx: int, assigned_trajs: List[tuple]):
        """Process assigned trajectories with specified optimization level."""
        api_key = client.api_keys[key_idx]
        
        # Setup worker logger
        log_dir = output_dir / "logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        worker_log_path = log_dir / f"worker_{key_idx:03d}.log"
        worker_logger = logging.getLogger(f"worker_{key_idx}")
        worker_handler = logging.FileHandler(worker_log_path)
        worker_handler.setFormatter(logging.Formatter('%(asctime)s [%(levelname)s] %(message)s'))
        worker_logger.addHandler(worker_handler)
        worker_logger.setLevel(logging.INFO)
        
        worker_logger.info(f"Worker {key_idx} started with {len(assigned_trajs)} trajectories (opt level {optimization_level})")
        
        # Create dedicated client
        key_config = APIConfig(
            provider=client.config.provider,
            model=client.config.model,
            api_key=api_key,
            base_url=client.config.base_url,
            temperature=client.config.temperature,
            max_tokens=client.config.max_tokens,
            timeout=client.config.timeout,
            rpm_limit=client.config.rpm_limit,
        )
        
        results = {}
        
        async with LLMClient(key_config, api_keys=[api_key]) as key_client:
            if optimization_level >= 3:
                # OPTIMIZATION 3: Process multiple trajectories in parallel
                semaphore = asyncio.Semaphore(trajectory_batch_size)
                
                async def process_with_semaphore(traj_id, steps, rel_path):
                    async with semaphore:
                        return await process_trajectory_parallel_steps(
                            traj_id, steps, rel_path, key_client, cache,
                            worker_logger, output_dir, write_lock,
                            store_raw_responses, step_batch_size
                        )
                
                # Process all trajectories with limited concurrency
                traj_tasks = [
                    process_with_semaphore(traj_id, steps, rel_path)
                    for traj_id, steps, rel_path in assigned_trajs
                ]
                
                traj_results = await asyncio.gather(*traj_tasks, return_exceptions=True)
                
                for (traj_id, _, _), result in zip(assigned_trajs, traj_results):
                    if isinstance(result, Exception):
                        worker_logger.error(f"Trajectory {traj_id} failed: {result}")
                    else:
                        results[traj_id] = result
            
            elif optimization_level == 2:
                # Process trajectories sequentially, steps in parallel
                for traj_id, steps, rel_path in assigned_trajs:
                    try:
                        enriched = await process_trajectory_parallel_steps(
                            traj_id, steps, rel_path, key_client, cache,
                            worker_logger, output_dir, write_lock,
                            store_raw_responses, step_batch_size
                        )
                        results[traj_id] = enriched
                    except Exception as e:
                        worker_logger.error(f"Trajectory {traj_id} failed: {e}")
            
            else:  # optimization_level == 1
                # Process trajectories and steps sequentially, axes in parallel
                for traj_idx, (traj_id, steps, rel_path) in enumerate(assigned_trajs):
                    try:
                        worker_logger.info(f"Starting trajectory {traj_idx+1}/{len(assigned_trajs)}: {traj_id}")
                        enriched_steps = []
                        
                        for i, step in enumerate(steps):
                            worker_logger.info(f"  Step {i+1}/{len(steps)}")
                            enriched = await process_single_step_parallel_axes(
                                step, i, enriched_steps, key_client, cache,
                                worker_logger, store_raw_responses
                            )
                            enriched_steps.append(enriched)
                        
                        # Write trajectory
                        output_path = output_dir / rel_path
                        output_path.parent.mkdir(parents=True, exist_ok=True)
                        
                        async with write_lock:
                            with open(output_path, 'w') as f:
                                for step in enriched_steps:
                                    f.write(json.dumps(step, default=str) + "\n")
                        
                        worker_logger.info(f"Completed trajectory {traj_id} ({len(enriched_steps)} steps)")
                        results[traj_id] = enriched_steps
                    
                    except Exception as e:
                        worker_logger.error(f"Trajectory {traj_id} failed: {e}")
        
        worker_logger.info(f"Worker {key_idx} finished: {len(results)} trajectories completed")
        worker_handler.close()
        worker_logger.removeHandler(worker_handler)
        
        return results
    
    # Process all workers in parallel
    tasks = [
        process_with_key(idx, assigned)
        for idx, assigned in enumerate(key_assignments)
        if assigned
    ]
    
    logger.info(f"Starting {len(tasks)} parallel workers")
    
    all_results = {}
    for coro in asyncio.as_completed(tasks):
        try:
            key_results = await coro
            all_results.update(key_results)
            logger.info(f"Worker finished, total trajectories: {len(all_results)}")
        except Exception as e:
            logger.error(f"Worker failed: {e}")
    
    return all_results


# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════

async def main_async(args):
    """Async main function."""
    # Load API keys
    api_keys = []
    env_path = Path('.env')
    if env_path.exists():
        with open(env_path) as f:
            api_keys = [line.strip() for line in f if line.strip() and line.strip().startswith('nvapi-')]
        if api_keys:
            logger.info(f"Loaded {len(api_keys)} API keys from .env file")
    
    if args.api_keys_file:
        with open(args.api_keys_file) as f:
            api_keys = [line.strip() for line in f if line.strip()]
        logger.info(f"Loaded {len(api_keys)} API keys from {args.api_keys_file}")
    
    if not api_keys and args.api_key:
        api_keys = [args.api_key]
    
    # Setup API client
    api_config = APIConfig(
        provider=args.provider,
        model=args.judge_model,
        api_key=api_keys[0] if api_keys else None,
        base_url=args.base_url,
        temperature=args.temperature,
        max_tokens=args.max_tokens,
        timeout=args.timeout,
        rpm_limit=args.rpm_limit,
    )
    
    # Setup cache
    cache_dir = Path(args.cache_dir)
    cache_dir.mkdir(parents=True, exist_ok=True)
    cache_filename = f"{args.judge_model.replace(':', '_').replace('/', '_')}.db"
    cache_path = cache_dir / cache_filename
    
    if args.clear_cache and cache_path.exists():
        logger.info(f"Clearing cache: {cache_path}")
        cache_path.unlink()
    
    cache = JudgeCache(cache_path)
    logger.info(f"Using cache: {cache_path}")
    
    # Load trajectories
    if args.input:
        trajectories_list = load_trajectories_from_jsonl(Path(args.input))
        trajectories = {tid: (steps, Path(f"{tid}.jsonl")) for tid, steps in trajectories_list.items()}
    elif args.input_dir:
        trajectories = load_trajectories_from_directory(Path(args.input_dir))
    else:
        logger.error("Must specify --input or --input_dir")
        sys.exit(1)
    
    logger.info(f"Loaded {len(trajectories)} trajectories")
    
    # Resume from checkpoint if output exists
    if args.output_dir and Path(args.output_dir).exists() and not args.overwrite:
        # Check which trajectory files already exist and are complete
        already_done = set()
        for traj_id, (steps, rel_path) in list(trajectories.items()):
            output_path = Path(args.output_dir) / rel_path
            if output_path.exists():
                # Verify file has complete trajectory
                try:
                    with open(output_path) as f:
                        lines = [json.loads(line) for line in f if line.strip()]
                    if len(lines) == len(steps) and all(s.get('judge_overall_score') is not None or s.get('judge_axis_scores') for s in lines):
                        already_done.add(traj_id)
                except Exception as e:
                    logger.warning(f"Could not verify {output_path}: {e}, will reprocess")
        
        trajectories = {tid: data for tid, data in trajectories.items() if tid not in already_done}
        logger.info(f"Resuming: {len(already_done)} done, {len(trajectories)} remaining")
    
    # Process trajectories
    async with LLMClient(api_config, api_keys=api_keys) as client:
        enriched = await process_trajectories_optimized(
            trajectories, client, cache,
            Path(args.output_dir) if args.output_dir else Path(args.output).parent,
            Path(args.input_dir) if args.input_dir else Path('.'),
            optimization_level=args.optimization_level,
            step_batch_size=args.step_batch_size,
            trajectory_batch_size=args.trajectory_batch_size,
            store_raw_responses=args.store_raw_responses
        )
        
        print_statistics(enriched, client, cache)
    
    cache.close()
    logger.info("Done!")


def main():
    parser = argparse.ArgumentParser(
        description="Optimized LLM judge evaluations",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    
    # Input/output
    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument("--input", type=Path)
    input_group.add_argument("--input_dir", type=Path)
    
    output_group = parser.add_mutually_exclusive_group(required=True)
    output_group.add_argument("--output", type=Path)
    output_group.add_argument("--output_dir", type=Path)
    
    # API configuration
    parser.add_argument("--provider", type=str, default="nvidia")
    parser.add_argument("--judge_model", type=str, default="nvidia/llama-3.1-nemotron-ultra-253b-v1")
    parser.add_argument("--api_key", type=str)
    parser.add_argument("--api_keys_file", type=Path)
    parser.add_argument("--base_url", type=str)
    parser.add_argument("--temperature", type=float, default=0.0)
    parser.add_argument("--max_tokens", type=int, default=4096)
    parser.add_argument("--timeout", type=int, default=240)
    parser.add_argument("--rpm_limit", type=int, default=35)
    
    # Optimization settings
    parser.add_argument(
        "--optimization_level", type=int, default=1, choices=[1, 2, 3],
        help="1=parallel axes (2.4x), 2=+parallel steps (9.6x), 3=+parallel trajectories (24x)"
    )
    parser.add_argument(
        "--step_batch_size", type=int, default=5,
        help="Number of steps to process in parallel (level 2+)"
    )
    parser.add_argument(
        "--trajectory_batch_size", type=int, default=10,
        help="Number of trajectories to process in parallel (level 3)"
    )
    
    # Other settings
    parser.add_argument("--max_concurrent", type=int, default=10)
    parser.add_argument("--cache_dir", type=Path, default=Path(".judge_cache"))
    parser.add_argument("--store_raw_responses", action="store_true")
    parser.add_argument("--clear_cache", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    
    args = parser.parse_args()
    
    logging.basicConfig(
        level=logging.INFO if args.verbose else logging.WARNING,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        stream=sys.stderr,
    )
    
    asyncio.run(main_async(args))


if __name__ == "__main__":
    main()
