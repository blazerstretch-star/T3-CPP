#!/usr/bin/env python3
"""
Work-Stealing Scheduler for LLM Judge Evaluation

Implements dynamic load balancing:
- Workers take from a shared queue
- When a worker finishes, it immediately gets new work
- No worker sits idle while others are busy
- Optimal utilization of all 109 API keys
"""

import asyncio
import json
import logging
import time
from collections import deque
from pathlib import Path
from typing import Dict, List, Tuple

# BUGFIX #7: Move imports to top level
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_calibrated import (
    route_evaluation_axes,
    get_axis_weights,
    aggregate_axis_scores,
    compute_hybrid_label
)
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_evaluator import evaluate_step, compute_cache_key, APIConfig, LLMClient


class WorkStealingQueue:
    """Thread-safe work queue with progress tracking."""
    
    def __init__(self, trajectories: Dict[str, Tuple]):
        self.queue = deque(trajectories.items())
        self.total = len(self.queue)
        self.completed = 0
        self.failed = 0  # BUGFIX #3: Track failed trajectories separately
        self.lock = asyncio.Lock()
        self.start_time = time.time()
    
    async def get_work(self) -> Tuple[str, Tuple]:
        """Get next trajectory to process (thread-safe)."""
        async with self.lock:
            if self.queue:
                return self.queue.popleft()
            return None, None
    
    async def mark_completed(self, failed: bool = False):
        """Mark a trajectory as completed and update progress."""
        # BUGFIX #8: Move print outside lock to reduce contention
        async with self.lock:
            self.completed += 1
            if failed:
                self.failed += 1  # BUGFIX #3: Track failures
            elapsed = time.time() - self.start_time
            progress_pct = (self.completed / self.total) * 100
            remaining = self.total - self.completed
            queue_len = len(self.queue)
            
            # Calculate ETA
            if self.completed > 0:
                eta_seconds = (elapsed / self.completed) * remaining
                eta_minutes = int(eta_seconds // 60)
                eta_hours = int(eta_minutes // 60)
                eta_minutes = eta_minutes % 60
            else:
                eta_hours = eta_minutes = 0
            
            # Snapshot data for printing
            snapshot = (self.completed, self.total, progress_pct, elapsed, eta_hours, eta_minutes, queue_len, self.failed)
        
        # BUGFIX #8: Print outside lock
        completed, total, progress_pct, elapsed, eta_hours, eta_minutes, queue_len, failed_count = snapshot
        bar_length = 50
        filled = int(bar_length * completed / total)
        bar = '█' * filled + '░' * (bar_length - filled)
        
        # BUGFIX #3: Show failed count in progress
        failed_str = f" | Failed: {failed_count}" if failed_count > 0 else ""
        print(f"\r[{bar}] {completed}/{total} ({progress_pct:.1f}%) | "
              f"Elapsed: {int(elapsed//60)}m {int(elapsed%60)}s | "
              f"ETA: {eta_hours}h {eta_minutes}m | "
              f"Queue: {queue_len} remaining{failed_str}", 
              end='', flush=True)
    
    # BUGFIX #5: Removed is_empty() - racy and unused
    
    async def get_stats(self) -> Dict:
        """Get current statistics (async-safe)."""
        # BUGFIX #10: Make async-safe
        async with self.lock:
            elapsed = time.time() - self.start_time
            return {
                'total': self.total,
                'completed': self.completed,
                'failed': self.failed,  # BUGFIX #3: Include failed count
                'remaining': len(self.queue),
                'elapsed': elapsed,
                'avg_time': elapsed / self.completed if self.completed > 0 else 0
            }


async def worker_with_work_stealing(
    worker_id: int,
    api_key: str,
    work_queue: WorkStealingQueue,
    client,
    cache,
    output_dir,
    store_raw_responses: bool,
    step_batch_size: int,
    write_lock: asyncio.Lock
):
    """
    Worker that continuously pulls from shared queue until empty.
    
    This is the key to work stealing - workers don't have pre-assigned work,
    they take from a shared queue dynamically.
    """
    # BUGFIX #7: Imports moved to top level
    
    # Setup worker logger
    log_dir = output_dir / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    worker_log_path = log_dir / f"worker_{worker_id:03d}.log"
    worker_logger = logging.getLogger(f"worker_{worker_id}")
    worker_handler = logging.FileHandler(worker_log_path)
    worker_handler.setFormatter(logging.Formatter('%(asctime)s [%(levelname)s] %(message)s'))
    worker_logger.addHandler(worker_handler)
    worker_logger.setLevel(logging.INFO)
    
    # Create dedicated client for this worker
    key_config = APIConfig(
        provider=client.config.provider,
        model=client.config.model,
        api_key=api_key,
        base_url=client.config.base_url,
        temperature=client.config.temperature,
        max_tokens=client.config.max_tokens,
        timeout=client.config.timeout,
        rpm_limit=client.config.rpm_limit,
        fallback_model=client.config.fallback_model,
        fallback_temperature=client.config.fallback_temperature,
        fallback_max_tokens=client.config.fallback_max_tokens,
    )
    
    trajectories_processed = 0
    
    # BUGFIX #4: Wrap in try/finally for handler cleanup
    try:
        async with LLMClient(key_config, api_keys=[api_key]) as worker_client:
            worker_logger.info(f"Worker {worker_id} started (work-stealing mode)")
            
            while True:
                # Get next work item from shared queue
                traj_id, traj_data = await work_queue.get_work()
                
                if traj_id is None:
                    # Queue is empty, worker is done
                    break
                
                steps, rel_path = traj_data
                trajectories_processed += 1
                
                try:
                    worker_logger.info(f"Processing trajectory {traj_id} ({len(steps)} steps)")
                    enriched_steps = []
                    
                    output_path = output_dir / rel_path
                    output_path.parent.mkdir(parents=True, exist_ok=True)
                    
                    # Process steps in batches
                    for batch_start in range(0, len(steps), step_batch_size):
                        batch_end = min(batch_start + step_batch_size, len(steps))
                        
                        # BUGFIX #2: Snapshot context before batch
                        context_snapshot = list(enriched_steps)
                        
                        async def process_single_step(step_idx: int, step: Dict):
                            """Process a single step with parallel axis evaluation."""
                            axes = route_evaluation_axes(step)
                            weights = get_axis_weights(axes, step)
                            
                            # Evaluate all axes in parallel
                            axis_scores = {}
                            axis_failures = {}
                            
                            async def evaluate_axis_with_retry(axis: str, max_retries: int = 2):
                                import aiohttp  # For ClientResponseError
                                last_exception = None
                                for attempt in range(max_retries):
                                    try:
                                        # BUGFIX #2: Use context_snapshot
                                        result = await evaluate_step(step, axis, worker_client, cache, context_snapshot)
                                        if result is not None:
                                            return result
                                        return None
                                    except (asyncio.TimeoutError, ConnectionError) as e:
                                        last_exception = e
                                        if attempt < max_retries - 1:
                                            wait_time = 2 ** attempt
                                            await asyncio.sleep(wait_time)
                                        continue
                                    except aiohttp.ClientResponseError as e:
                                        # Retry 429/503/502 errors
                                        if e.status in (429, 503, 502):
                                            last_exception = e
                                            if attempt < max_retries - 1:
                                                wait_time = 2 ** attempt
                                                await asyncio.sleep(wait_time)
                                            continue
                                        else:
                                            worker_logger.error(f"Axis {axis} HTTP {e.status}: {str(e)}")
                                            raise
                                    except Exception as e:
                                        worker_logger.error(f"Axis {axis} failed: {type(e).__name__}")
                                        raise
                                
                                if last_exception:
                                    raise last_exception
                                return None
                            
                            # Parallel axis evaluation
                            axis_tasks = {axis: evaluate_axis_with_retry(axis) for axis in axes}
                            axis_results = await asyncio.gather(*axis_tasks.values(), return_exceptions=True)
                            
                            for axis, result in zip(axis_tasks.keys(), axis_results):
                                if isinstance(result, Exception):
                                    axis_failures[axis] = f"{type(result).__name__}"
                                elif result is not None:
                                    axis_scores[axis] = result
                            
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
                                    # BUGFIX #2: Use context_snapshot for cache key
                                    cache_key = compute_cache_key(step, axis, worker_client.config.model, context_snapshot)
                                    cached = cache.get(cache_key)
                                    if cached and 'justification' in cached:
                                        enriched_step['judge_axis_justifications'][axis] = cached['justification']
                            
                            enriched_step.update(judge_result)
                            enriched_step.update(hybrid_label)
                            
                            return enriched_step
                    
                    # Process batch in parallel
                    batch_steps = steps[batch_start:batch_end]
                    step_tasks = [
                        process_single_step(batch_start + i, step)
                        for i, step in enumerate(batch_steps)
                    ]
                    
                    batch_results = await asyncio.gather(*step_tasks, return_exceptions=True)
                        
                        # Use zip for robust pairing
                        for step, result in zip(batch_steps, batch_results):
                            if isinstance(result, Exception):
                                worker_logger.error(f"Step {step.get('step_position', '?')+1} failed: {result}")
                                enriched_steps.append({**step, 'judge_overall_score': None, 'judge_failed': True})
                            else:
                                enriched_steps.append(result)
                    
                    # Write trajectory
                    if output_path and enriched_steps:
                        async with write_lock:
                            with open(output_path, 'w') as f:
                                for step in enriched_steps:
                                    f.write(json.dumps(step, default=str) + "\n")
                        
                        worker_logger.info(f"Completed trajectory {traj_id} ({len(enriched_steps)} steps)")
                    
                    # Mark as completed (updates progress bar)
                    await work_queue.mark_completed(failed=False)
                    
                except Exception as e:
                    worker_logger.error(f"Failed trajectory {traj_id}: {type(e).__name__}: {str(e)}")
                    # BUGFIX #3: Mark as failed
                    await work_queue.mark_completed(failed=True)
            
            worker_logger.info(f"Worker {worker_id} finished: {trajectories_processed} trajectories processed")
    finally:
        # BUGFIX #4: Guaranteed handler cleanup
        try:
            worker_handler.close()
            worker_logger.removeHandler(worker_handler)
        except Exception as e:
            pass  # Ignore cleanup errors


async def process_with_work_stealing(
    trajectories: Dict[str, Tuple],
    client,
    cache,
    output_dir,
    store_raw_responses: bool = False,
    step_batch_size: int = 5
) -> Dict:
    """
    Process trajectories using work-stealing scheduler.
    
    All workers pull from a shared queue - no pre-assignment.
    Fast workers automatically get more work.
    """
    from pathlib import Path
    
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Create shared work queue
    work_queue = WorkStealingQueue(trajectories)
    write_lock = asyncio.Lock()
    
    print(f"\n{'='*70}")
    print(f"Work-Stealing Scheduler: {len(trajectories)} trajectories, {len(client.api_keys)} workers")
    print(f"{'='*70}\n")
    
    # Create worker tasks (one per API key)
    worker_tasks = [
        worker_with_work_stealing(
            worker_id=i,
            api_key=client.api_keys[i],
            work_queue=work_queue,
            client=client,
            cache=cache,
            output_dir=output_dir,
            store_raw_responses=store_raw_responses,
            step_batch_size=step_batch_size,
            write_lock=write_lock
        )
        for i in range(len(client.api_keys))
    ]
    
    # Run all workers in parallel
    await asyncio.gather(*worker_tasks)
    
    print("\n")  # New line after progress bar
    
    # Get final stats
    stats = await work_queue.get_stats()  # BUGFIX #10: await async method
    print(f"\n{'='*70}")
    print(f"Work-Stealing Completed!")
    print(f"Total trajectories: {stats['total']}")
    print(f"Completed: {stats['completed']}")
    print(f"Failed: {stats['failed']}")  # BUGFIX #3: Show failed count
    print(f"Total time: {int(stats['elapsed']//60)}m {int(stats['elapsed']%60)}s")
    print(f"Avg time per trajectory: {stats['avg_time']:.1f}s")
    print(f"{'='*70}\n")
    
    # BUGFIX #1: Return summary dict instead of empty dict
    return {
        'total': stats['total'],
        'completed': stats['completed'],
        'failed': stats['failed'],
        'elapsed': stats['elapsed']
    }


__all__ = ['process_with_work_stealing', 'WorkStealingQueue']
