#!/usr/bin/env python3
"""
verify_split_orjson.py

Verify train/val/test splits have zero overlap in:
1. Trajectory IDs
2. Task IDs (for test set)
3. Repositories (for test set)

OPTIMIZED: Uses orjson for 2-3x faster loading on large files
"""

import json
from pathlib import Path
from collections import defaultdict
from tqdm import tqdm

# Try importing orjson for speed
try:
    import orjson
    USE_ORJSON = True
    print("✅ Using orjson (fast JSON parsing)")
except ImportError:
    USE_ORJSON = False
    print("⚠️  orjson not found, using standard json (slower)")
    print("   Install with: pip install orjson")

def load_split_metadata(split_file):
    """Load trajectory IDs, task IDs, and repos from a split"""
    trajectory_ids = set()
    task_ids = set()
    repos = set()
    step_count = 0
    
    # Get file size for progress bar
    file_size = Path(split_file).stat().st_size
    file_size_mb = file_size / (1024 * 1024)
    
    print(f"\nLoading {split_file.name} ({file_size_mb:.1f} MB)...")
    
    if USE_ORJSON:
        # orjson requires binary mode
        with open(split_file, 'rb') as f:
            for line in tqdm(f, desc=f"  Parsing {split_file.name}", unit=" lines"):
                try:
                    step = orjson.loads(line)
                    trajectory_ids.add(step['trajectory_id'])
                    task_ids.add(step['task_id'])
                    repos.add(step.get('repo', 'unknown'))
                    step_count += 1
                except Exception as e:
                    print(f"    ⚠️  Skipping malformed line: {e}")
                    continue
    else:
        # Standard json with text mode
        with open(split_file, 'r') as f:
            for line in tqdm(f, desc=f"  Parsing {split_file.name}", unit=" lines"):
                try:
                    step = json.loads(line)
                    trajectory_ids.add(step['trajectory_id'])
                    task_ids.add(step['task_id'])
                    repos.add(step.get('repo', 'unknown'))
                    step_count += 1
                except Exception as e:
                    print(f"    ⚠️  Skipping malformed line: {e}")
                    continue
    
    return {
        'trajectory_ids': trajectory_ids,
        'task_ids': task_ids,
        'repos': repos,
        'step_count': step_count
    }

def verify_splits(output_dir):
    """Verify no overlap between splits"""
    
    output_path = Path(output_dir)
    
    print("="*80)
    print("SPLIT VERIFICATION (HELD-OUT VALIDATION)")
    print("="*80)
    
    # Check files exist
    train_file = output_path / "train.jsonl"
    val_file = output_path / "val.jsonl"
    test_file = output_path / "test.jsonl"
    
    print("\n📁 DATA PATHS:")
    print(f"  Train: {train_file.absolute()}")
    print(f"  Val:   {val_file.absolute()}")
    print(f"  Test:  {test_file.absolute()}")
    
    print("\n📊 FILE EXISTENCE CHECK:")
    all_exist = True
    for f in [train_file, val_file, test_file]:
        if not f.exists():
            print(f"  ❌ File not found: {f}")
            all_exist = False
        else:
            file_size_mb = f.stat().st_size / (1024 * 1024)
            print(f"  ✅ {f.name}: {file_size_mb:.2f} MB")
    
    if not all_exist:
        return False
    
    # Load metadata
    print("\nLoading split metadata...")
    train = load_split_metadata(train_file)
    val = load_split_metadata(val_file)
    test = load_split_metadata(test_file)
    
    # Print summary
    print("\n" + "="*80)
    print("SPLIT SUMMARY")
    print("="*80)
    
    print(f"\nTRAIN:")
    print(f"  Steps: {train['step_count']:,}")
    print(f"  Trajectories: {len(train['trajectory_ids']):,}")
    print(f"  Tasks: {len(train['task_ids']):,}")
    print(f"  Repositories: {len(train['repos']):,}")
    print(f"  Repos: {', '.join(sorted(train['repos']))}")
    
    print(f"\nVAL:")
    print(f"  Steps: {val['step_count']:,}")
    print(f"  Trajectories: {len(val['trajectory_ids']):,}")
    print(f"  Tasks: {len(val['task_ids']):,}")
    print(f"  Repositories: {len(val['repos']):,}")
    print(f"  Repos: {', '.join(sorted(val['repos']))}")
    
    print(f"\nTEST:")
    print(f"  Steps: {test['step_count']:,}")
    print(f"  Trajectories: {len(test['trajectory_ids']):,}")
    print(f"  Tasks: {len(test['task_ids']):,}")
    print(f"  Repositories: {len(test['repos']):,}")
    print(f"  Repos: {', '.join(sorted(test['repos']))}")
    
    # Check overlaps
    print("\n" + "="*80)
    print("OVERLAP VERIFICATION (HELD-OUT CHECKS)")
    print("="*80)
    
    all_passed = True
    
    # 1. Trajectory ID overlap (CRITICAL: must be zero)
    print("\n1. TRAJECTORY ID OVERLAP:")
    train_val_traj = train['trajectory_ids'] & val['trajectory_ids']
    train_test_traj = train['trajectory_ids'] & test['trajectory_ids']
    val_test_traj = val['trajectory_ids'] & test['trajectory_ids']
    
    if train_val_traj:
        print(f"  ❌ Train-Val overlap: {len(train_val_traj):,} trajectories")
        print(f"     CRITICAL: Val trajectories leak into training!")
        all_passed = False
    else:
        print(f"  ✅ Train-Val: No overlap ({len(train['trajectory_ids']):,} vs {len(val['trajectory_ids']):,})")
    
    if train_test_traj:
        print(f"  ❌ Train-Test overlap: {len(train_test_traj):,} trajectories")
        print(f"     CRITICAL: Test trajectories leak into training!")
        all_passed = False
    else:
        print(f"  ✅ Train-Test: No overlap ({len(train['trajectory_ids']):,} vs {len(test['trajectory_ids']):,})")
    
    if val_test_traj:
        print(f"  ❌ Val-Test overlap: {len(val_test_traj):,} trajectories")
        print(f"     CRITICAL: Test trajectories leak into validation!")
        all_passed = False
    else:
        print(f"  ✅ Val-Test: No overlap ({len(val['trajectory_ids']):,} vs {len(test['trajectory_ids']):,})")
    
    # 2. Repository overlap (test should be disjoint for zero-shot generalization)
    print("\n2. REPOSITORY OVERLAP (Zero-Shot Generalization):")
    train_test_repos = train['repos'] & test['repos']
    val_test_repos = val['repos'] & test['repos']
    
    if train_test_repos:
        print(f"  ❌ Train-Test repo overlap: {train_test_repos}")
        print(f"     WARNING: Test repos seen during training (not zero-shot)")
        all_passed = False
    else:
        print(f"  ✅ Train-Test repos: No overlap (zero-shot generalization)")
        print(f"     Train repos: {sorted(train['repos'])}")
        print(f"     Test repos:  {sorted(test['repos'])}")
    
    if val_test_repos:
        print(f"  ❌ Val-Test repo overlap: {val_test_repos}")
        all_passed = False
    else:
        print(f"  ✅ Val-Test repos: No overlap")
    
    # Val should be subset of train repos (same distribution)
    if val['repos'] <= train['repos']:
        print(f"  ✅ Val repos are subset of train repos (same distribution)")
    else:
        print(f"  ⚠️  Val has repos not in train: {val['repos'] - train['repos']}")
        print(f"     This may indicate distribution mismatch")
    
    # 3. Task overlap (test tasks should be disjoint for generalization)
    print("\n3. TASK OVERLAP (Task-Level Generalization):")
    train_test_tasks = train['task_ids'] & test['task_ids']
    val_test_tasks = val['task_ids'] & test['task_ids']
    train_val_tasks = train['task_ids'] & val['task_ids']
    
    if train_test_tasks:
        print(f"  ❌ Train-Test task overlap: {len(train_test_tasks):,} tasks")
        print(f"     WARNING: Test tasks seen during training")
        all_passed = False
    else:
        print(f"  ✅ Train-Test tasks: No overlap ({len(train['task_ids']):,} vs {len(test['task_ids']):,})")
    
    if val_test_tasks:
        print(f"  ❌ Val-Test task overlap: {len(val_test_tasks):,} tasks")
        all_passed = False
    else:
        print(f"  ✅ Val-Test tasks: No overlap ({len(val['task_ids']):,} vs {len(test['task_ids']):,})")
    
    # Val tasks overlap with train is EXPECTED (same task, different trajectories)
    if train_val_tasks:
        overlap_pct = len(train_val_tasks) / len(val['task_ids']) * 100
        print(f"  ℹ️  Train-Val task overlap: {len(train_val_tasks):,} tasks ({overlap_pct:.1f}% of val)")
        print(f"     This is EXPECTED: Val uses same tasks but different trajectories")
    else:
        print(f"  ⚠️  Train-Val tasks: No overlap (unusual - val should share tasks)")
    
    # 4. Coverage check
    print("\n4. COVERAGE CHECK:")
    total_trajectories = len(train['trajectory_ids'] | val['trajectory_ids'] | test['trajectory_ids'])
    total_steps = train['step_count'] + val['step_count'] + test['step_count']
    
    print(f"  Total unique trajectories: {total_trajectories:,}")
    print(f"  Total steps: {total_steps:,}")
    print(f"  Train: {train['step_count']:,} steps ({train['step_count']/total_steps*100:.1f}%)")
    print(f"  Val:   {val['step_count']:,} steps ({val['step_count']/total_steps*100:.1f}%)")
    print(f"  Test:  {test['step_count']:,} steps ({test['step_count']/total_steps*100:.1f}%)")
    
    # Typical split ratios
    train_pct = train['step_count'] / total_steps * 100
    val_pct = val['step_count'] / total_steps * 100
    test_pct = test['step_count'] / total_steps * 100
    
    if 70 <= train_pct <= 85:
        print(f"  ✅ Train ratio ({train_pct:.1f}%) is reasonable (70-85%)")
    else:
        print(f"  ⚠️  Train ratio ({train_pct:.1f}%) is unusual (expected 70-85%)")
    
    if 5 <= val_pct <= 15:
        print(f"  ✅ Val ratio ({val_pct:.1f}%) is reasonable (5-15%)")
    else:
        print(f"  ⚠️  Val ratio ({val_pct:.1f}%) is unusual (expected 5-15%)")
    
    if 10 <= test_pct <= 25:
        print(f"  ✅ Test ratio ({test_pct:.1f}%) is reasonable (10-25%)")
    else:
        print(f"  ⚠️  Test ratio ({test_pct:.1f}%) is unusual (expected 10-25%)")
    
    # Final verdict
    print("\n" + "="*80)
    if all_passed:
        print("✅ ALL CHECKS PASSED - Split is valid for held-out evaluation!")
        print("\nKey validations:")
        print("  ✓ Zero trajectory overlap (no data leakage)")
        print("  ✓ Zero repository overlap in test (zero-shot generalization)")
        print("  ✓ Zero task overlap in test (task-level generalization)")
    else:
        print("❌ SOME CHECKS FAILED - Review overlap issues above")
        print("\nCRITICAL: Data leakage detected! Fix before training.")
    print("="*80)
    
    return all_passed

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Verify train/val/test split integrity")
    parser.add_argument(
        '--data_dir',
        type=str,
        default="path/t3_cpp/data_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train",
        help="Directory containing train.jsonl, val.jsonl, test.jsonl"
    )
    args = parser.parse_args()
    
    success = verify_splits(args.data_dir)
    exit(0 if success else 1)

if __name__ == "__main__":
    main()
