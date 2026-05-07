#!/usr/bin/env python3
"""
LLM Judge with Write-Ahead Log (WAL) and Checkpoint Support

Features:
- Fast append-only WAL writes (no I/O blocking)
- Atomic operations (crash-safe)
- Resume capability (skip completed trajectories)
- Progress tracking
- Separate conversion phase (WAL → directory structure)

Usage:
    # Phase 1: Evaluation with WAL
    python add_llm_judge_wal.py \
        --input_dir data_FINAL_TUE_02_20_PM \
        --wal_file results/evaluation.wal \
        --checkpoint_file results/evaluation.checkpoint \
        --sample_rate 0.0011
    
    # Phase 2: Convert WAL to directory structure
    python add_llm_judge_wal.py \
        --convert_wal results/evaluation.wal \
        --output_dir data_FINAL_TUE_02_20_PM_judged_150
    
    # Resume after crash (automatically skips completed)
    python add_llm_judge_wal.py \
        --input_dir data_FINAL_TUE_02_20_PM \
        --wal_file results/evaluation.wal \
        --checkpoint_file results/evaluation.checkpoint \
        --sample_rate 0.0011 \
        --resume
"""

import argparse
import asyncio
import json
import logging
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, List, Set
from datetime import datetime
import fcntl

from tqdm import tqdm

# Import from existing modules
from code_and_train_ds.t3_cpp.core.llm_judge_framework.add_llm_judge import (
    load_trajectories_from_directory,
    LLMClient,
    APIConfig,
    JudgeCache,
    evaluate_step,
    route_evaluation_axes,
    get_axis_weights,
    aggregate_axis_scores,
    compute_hybrid_label,
    compute_cache_key,
    print_statistics,
)

logger = logging.getLogger("add_llm_judge_wal")


# ═══════════════════════════════════════════════════════════════════════════
# WAL OPERATIONS
# ═══════════════════════════════════════════════════════════════════════════

class WALWriter:
    """Thread-safe Write-Ahead Log writer with atomic appends."""
    
    def __init__(self, wal_path: Path):
        self.wal_path = wal_path
        self.wal_path.parent.mkdir(parents=True, exist_ok=True)
        self.lock = asyncio.Lock()
    
    async def append(self, trajectory_id: str, steps: List[Dict], rel_path: Path):
        """Atomically append trajectory to WAL."""
        entry = {
            "trajectory_id": trajectory_id,
            "steps": steps,
            "rel_path": str(rel_path),
            "timestamp": datetime.now().isoformat(),
        }
        
        async with self.lock:
            # Atomic append with file locking
            with open(self.wal_path, 'a') as f:
                fcntl.flock(f.fileno(), fcntl.LOCK_EX)
                try:
                    f.write(json.dumps(entry, default=str) + '\n')
                    f.flush()
                finally:
                    fcntl.flock(f.fileno(), fcntl.LOCK_UN)
    
    def read_completed_ids(self) -> Set[str]:
        """Read all completed trajectory IDs from WAL."""
        if not self.wal_path.exists():
            return set()
        
        completed = set()
        with open(self.wal_path) as f:
            for line in f:
                if line.strip():
                    try:
                        entry = json.loads(line)
                        completed.add(entry['trajectory_id'])
                    except json.JSONDecodeError:
                        logger.warning(f"Skipping malformed WAL line: {line[:100]}")
        
        return completed
    
    def count_entries(self) -> int:
        """Count entries in WAL for progress tracking."""
        if not self.wal_path.exists():
            return 0
        
        with open(self.wal_path) as f:
            return sum(1 for line in f if line.strip())


class CheckpointManager:
    """Manages checkpoint file for progress tracking."""
    
    def __init__(self, checkpoint_path: Path):
        self.checkpoint_path = checkpoint_path
        self.checkpoint_path.parent.mkdir(parents=True, exist_ok=True)
        self.lock = asyncio.Lock()
    
    async def update(self, completed_ids: Set[str], total: int):
        """Update checkpoint file atomically."""
        checkpoint = {
            "completed": sorted(list(completed_ids)),
            "total": total,
            "progress": len(completed_ids),
            "last_updated": datetime.now().isoformat(),
        }
        
        async with self.lock:
            # Write to temp file, then atomic rename
            temp_path = self.checkpoint_path.with_suffix('.tmp')
            with open(temp_path, 'w') as f:
                json.dump(checkpoint, f, indent=2)
                f.flush()
            
            temp_path.replace(self.checkpoint_path)
    
    def load(self) -> Set[str]:
        """Load completed trajectory IDs from checkpoint."""
        if not self.checkpoint_path.exists():
            return set()
        
        try:
            with open(self.checkpoint_path) as f:
                checkpoint = json.load(f)
                return set(checkpoint.get('completed', []))
        except (json.JSONDecodeError, KeyError):
            logger.warning("Checkpoint file corrupted, starting fresh")
            return set()
    
    def get_progress(self) -> Dict:
        """Get current progress statistics."""
        if not self.checkpoint_path.exists():
            return {"progress": 0, "total": 0, "percent": 0.0}
        
        try:
            with open(self.checkpoint_path) as f:
                checkpoint = json.load(f)
                progress = checkpoint.get('progress', 0)
                total = checkpoint.get('total', 0)
                return {
                    "progress": progress,
                    "total": total,
                    "percent": 100 * progress / total if total > 0 else 0.0,
                    "last_updated": checkpoint.get('last_updated'),
                }
        except:
            return {"progress": 0, "total": 0, "percent": 0.0}


# ═══════════════════════════════════════════════════════════════════════════
# EVALUATION WITH WAL
# ═══════════════════════════════════════════════════════════════════════════

async def evaluate_with_wal(
    trajectories: Dict[str, tuple],
    client: LLMClient,
    cache: JudgeCache,
    wal_writer: WALWriter,
    checkpoint_mgr: CheckpointManager,
    max_concurrent: int = 10,
    step_batch_size: int = 5,
    checkpoint_interval: int = 10,
) -> Dict[str, List[Dict]]:
    """
    Evaluate trajectories with WAL and checkpoint support.
    
    Returns:
        Dict mapping trajectory_id to enriched steps
    """
    
    # Track completed trajectories
    completed_ids = set()
    completed_count = 0
    total_trajectories = len(trajectories)
    
    # Distribute trajectories across API keys
    num_keys = len(client.api_keys)
    key_assignments = [[] for _ in range(num_keys)]
    
    for idx, (traj_id, (steps, rel_path)) in enumerate(trajectories.items()):
        key_idx = idx % num_keys
        key_assignments[key_idx].append((traj_id, steps, rel_path))
    
    logger.info(f"Distributed {len(trajectories)} trajectories across {num_keys} API keys")
    logger.info(f"OPTIMIZATION: Parallel axes + parallel steps (batch_size={step_batch_size})")
    
    async def process_with_key(key_idx: int, assigned_trajs: List[tuple]):
        """Process assigned trajectories with one API key."""
        nonlocal completed_count
        
        api_key = client.api_keys[key_idx]
        
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
        
        results = {}
        
        async with LLMClient(key_config, api_keys=[api_key]) as key_client:
            for traj_idx, (traj_id, steps, rel_path) in enumerate(assigned_trajs):
                try:
                    enriched_steps = []
                    
                    # Process steps in batches (parallel)
                    for batch_start in range(0, len(steps), step_batch_size):
                        batch_end = min(batch_start + step_batch_size, len(steps))
                        batch_steps = steps[batch_start:batch_end]
                        
                        # Process batch in parallel
                        async def process_single_step(step: Dict):
                            axes = route_evaluation_axes(step)
                            weights = get_axis_weights(axes, step)
                            
                            # Evaluate all axes in parallel
                            axis_tasks = {
                                axis: evaluate_step(step, axis, key_client, cache, enriched_steps)
                                for axis in axes
                            }
                            
                            axis_results = await asyncio.gather(
                                *axis_tasks.values(),
                                return_exceptions=True
                            )
                            
                            # Process results
                            axis_scores = {}
                            for axis, result in zip(axis_tasks.keys(), axis_results):
                                if not isinstance(result, Exception) and result is not None:
                                    axis_scores[axis] = result
                            
                            # Aggregate scores
                            judge_result = aggregate_axis_scores(axis_scores, weights)
                            hybrid_label = compute_hybrid_label(step, judge_result)
                            
                            # Build enriched step
                            enriched_step = {**step}
                            enriched_step['judge_axes_used'] = axes
                            enriched_step['judge_axis_scores'] = axis_scores
                            enriched_step.update(judge_result)
                            enriched_step.update(hybrid_label)
                            
                            return enriched_step
                        
                        # Process batch
                        step_tasks = [process_single_step(step) for step in batch_steps]
                        batch_results = await asyncio.gather(*step_tasks, return_exceptions=True)
                        
                        for result in batch_results:
                            if not isinstance(result, Exception):
                                enriched_steps.append(result)
                    
                    # Write to WAL (fast, atomic)
                    await wal_writer.append(traj_id, enriched_steps, rel_path)
                    
                    # Update checkpoint periodically
                    completed_ids.add(traj_id)
                    completed_count += 1
                    
                    if completed_count % checkpoint_interval == 0:
                        await checkpoint_mgr.update(completed_ids, total_trajectories)
                        logger.info(f"Progress: {completed_count}/{total_trajectories} ({100*completed_count/total_trajectories:.1f}%)")
                    
                    results[traj_id] = enriched_steps
                    
                except Exception as e:
                    logger.error(f"Failed trajectory {traj_id}: {type(e).__name__}: {str(e)}")
        
        return results
    
    # Process all key assignments in parallel
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
        except Exception as e:
            logger.error(f"Worker failed: {type(e).__name__}: {str(e)}")
    
    # Final checkpoint update
    await checkpoint_mgr.update(completed_ids, total_trajectories)
    
    return all_results


# ═══════════════════════════════════════════════════════════════════════════
# WAL CONVERSION
# ═══════════════════════════════════════════════════════════════════════════

def convert_wal_to_directory(wal_path: Path, output_dir: Path):
    """Convert WAL file to directory structure."""
    
    if not wal_path.exists():
        logger.error(f"WAL file not found: {wal_path}")
        return
    
    output_dir.mkdir(parents=True, exist_ok=True)
    
    print(f"\n🔄 Converting WAL to directory structure...")
    print(f"  WAL file: {wal_path}")
    print(f"  Output:   {output_dir}")
    
    # Count entries for progress bar
    total_entries = 0
    with open(wal_path) as f:
        total_entries = sum(1 for line in f if line.strip())
    
    # Convert entries
    converted = 0
    with open(wal_path) as f:
        for line in tqdm(f, total=total_entries, desc="Converting"):
            if not line.strip():
                continue
            
            try:
                entry = json.loads(line)
                traj_id = entry['trajectory_id']
                steps = entry['steps']
                rel_path = Path(entry['rel_path'])
                
                # Write to output directory
                output_path = output_dir / rel_path
                output_path.parent.mkdir(parents=True, exist_ok=True)
                
                with open(output_path, 'w') as out_f:
                    for step in steps:
                        out_f.write(json.dumps(step, default=str) + '\n')
                
                converted += 1
                
            except Exception as e:
                logger.error(f"Failed to convert entry: {e}")
    
    print(f"\n✅ Converted {converted}/{total_entries} trajectories")
    print(f"  Output directory: {output_dir}")


# ═══════════════════════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════════════════════

async def main_async(args):
    """Async main function."""
    
    # Mode 1: Convert WAL to directory
    if args.convert_wal:
        convert_wal_to_directory(Path(args.convert_wal), Path(args.output_dir))
        return
    
    # Mode 2: Evaluation with WAL
    
    # Load API keys
    api_keys = []
    # Try multiple locations for .env file
    env_paths = [
        Path('.env'),
        Path('../../.env'),
        Path(__file__).parent.parent.parent / '.env',
    ]
    
    env_path = None
    for path in env_paths:
        if path.exists():
            env_path = path
            break
    
    if env_path and env_path.exists():
        with open(env_path) as f:
            api_keys = [line.strip() for line in f if line.strip() and line.strip().startswith('nvapi-')]
        logger.info(f"Found .env file with {len(api_keys)} API keys at {env_path}")
    
    if not api_keys:
        logger.error(f"No API keys found in .env file. Tried: {[str(p) for p in env_paths]}")
        sys.exit(1)
    
    logger.info(f"Loaded {len(api_keys)} API keys from .env file")
    
    # Setup API client
    api_config = APIConfig(
        provider=args.provider,
        model=args.judge_model,
        api_key=api_keys[0],
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
    cache = JudgeCache(cache_path)
    
    logger.info(f"Using cache: {cache_path}")
    logger.info(f"Cache contains {cache.stats()['total']} entries")
    
    # Setup WAL and checkpoint
    wal_writer = WALWriter(Path(args.wal_file))
    checkpoint_mgr = CheckpointManager(Path(args.checkpoint_file))
    
    # Load trajectories - ONLY from specific subdirectories
    logger.info(f"Loading trajectories from {args.input_dir}")
    
    # Filter to only load from long, short, medium, openai subdirectories
    input_base = Path(args.input_dir)
    allowed_subdirs = ['long', 'short', 'medium', 'openai']
    
    all_trajectories = {}
    for subdir in allowed_subdirs:
        subdir_path = input_base / subdir
        if subdir_path.exists():
            logger.info(f"Loading from subdirectory: {subdir}")
            subdir_trajs = load_trajectories_from_directory(subdir_path)
            logger.info(f"  Loaded {len(subdir_trajs)} trajectories from {subdir}")
            all_trajectories.update(subdir_trajs)
        else:
            logger.warning(f"Subdirectory not found: {subdir}")
    
    trajectories = all_trajectories
    logger.info(f"Total loaded: {len(trajectories)} trajectories from {len(allowed_subdirs)} subdirectories")
    
    # Sample if requested
    if args.sample_rate < 1.0:
        import random
        random.seed(args.seed)
        sample_size = int(len(trajectories) * args.sample_rate)
        traj_ids = random.sample(list(trajectories.keys()), sample_size)
        trajectories = {tid: trajectories[tid] for tid in traj_ids}
        logger.info(f"Sampled {len(trajectories)} trajectories ({args.sample_rate:.1%})")
    
    # Resume: skip completed trajectories
    if args.resume:
        completed = wal_writer.read_completed_ids()
        if completed:
            trajectories = {tid: data for tid, data in trajectories.items() if tid not in completed}
            logger.info(f"Resuming: {len(completed)} done, {len(trajectories)} remaining")
            
            # Show progress
            progress = checkpoint_mgr.get_progress()
            logger.info(f"Previous progress: {progress['progress']}/{progress['total']} ({progress['percent']:.1f}%)")
    
    # Evaluate with WAL
    async with LLMClient(api_config, api_keys=api_keys) as client:
        enriched = await evaluate_with_wal(
            trajectories, client, cache, wal_writer, checkpoint_mgr,
            args.max_concurrent, args.step_batch_size, args.checkpoint_interval
        )
        
        # Print statistics
        print_statistics(enriched, client, cache)
    
    cache.close()
    
    print(f"\n✅ Evaluation complete!")
    print(f"  WAL file: {args.wal_file}")
    print(f"  Checkpoint: {args.checkpoint_file}")
    print(f"  Entries: {wal_writer.count_entries()}")
    
    # Auto-convert if requested
    if args.auto_convert and args.output_dir:
        print(f"\n🔄 Auto-converting WAL to directory structure...")
        convert_wal_to_directory(Path(args.wal_file), Path(args.output_dir))
    else:
        print(f"\n💡 To convert WAL to directory structure, run:")
        print(f"  python {sys.argv[0]} --convert_wal {args.wal_file} --output_dir <output_dir>")


def main():
    parser = argparse.ArgumentParser(
        description="LLM Judge with WAL and Checkpoint Support",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    
    # Mode selection
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument(
        "--input_dir", type=Path,
        help="Input directory with trajectories (evaluation mode)"
    )
    mode_group.add_argument(
        "--convert_wal", type=Path,
        help="Convert WAL file to directory structure (conversion mode)"
    )
    
    # WAL and checkpoint
    parser.add_argument(
        "--wal_file", type=Path, default=Path("results/evaluation.wal"),
        help="Write-ahead log file path"
    )
    parser.add_argument(
        "--checkpoint_file", type=Path, default=Path("results/evaluation.checkpoint"),
        help="Checkpoint file path"
    )
    parser.add_argument(
        "--output_dir", type=Path,
        help="Output directory for final structure"
    )
    parser.add_argument(
        "--resume", action="store_true",
        help="Resume from checkpoint (skip completed trajectories)"
    )
    parser.add_argument(
        "--auto_convert", action="store_true",
        help="Automatically convert WAL to directory after evaluation"
    )
    
    # API configuration
    parser.add_argument(
        "--provider", type=str, default="nvidia",
        choices=["openai", "anthropic", "ollama", "nvidia"],
        help="LLM API provider"
    )
    parser.add_argument(
        "--judge_model", type=str, default="nvidia/llama-3.1-nemotron-ultra-253b-v1",
        help="Model for LLM judge"
    )
    parser.add_argument(
        "--base_url", type=str,
        help="Custom API base URL"
    )
    
    # API parameters
    parser.add_argument(
        "--temperature", type=float, default=0.0,
        help="LLM temperature"
    )
    parser.add_argument(
        "--max_tokens", type=int, default=4096,
        help="Max tokens per API call"
    )
    parser.add_argument(
        "--timeout", type=int, default=240,
        help="API timeout in seconds"
    )
    parser.add_argument(
        "--rpm_limit", type=int, default=35,
        help="Rate limit: requests per minute per API key"
    )
    
    # Performance
    parser.add_argument(
        "--max_concurrent", type=int, default=10,
        help="Max concurrent API calls"
    )
    parser.add_argument(
        "--step_batch_size", type=int, default=5,
        help="Number of steps to process in parallel"
    )
    parser.add_argument(
        "--checkpoint_interval", type=int, default=10,
        help="Update checkpoint every N trajectories"
    )
    parser.add_argument(
        "--cache_dir", type=Path, default=Path(".judge_cache"),
        help="Cache directory for LLM responses"
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
    
    # Run async main
    asyncio.run(main_async(args))


if __name__ == "__main__":
    main()
