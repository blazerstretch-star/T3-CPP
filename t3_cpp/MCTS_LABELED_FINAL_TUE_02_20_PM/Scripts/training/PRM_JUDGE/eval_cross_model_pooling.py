#!/usr/bin/env python3
"""
Cross-Model Trajectory Pooling Evaluation

Tests the Capability Orthogonality Hypothesis:
- Single-model: PRM selects best trajectory per model independently
- Cross-model: PRM selects best trajectory from pooled trajectories across all models
- Oracle: Best trajectory exists in any model's pool

Usage:
  python eval_cross_model_pooling.py \
    --checkpoint outputs_simple_labels_strong_ldca/final_model \
    --test_file test.jsonl \
    --output_dir results/cross_model_pooling
"""

import json
import os
try:
    import orjson
    USE_ORJSON = True
except ImportError:
    USE_ORJSON = False
    print("⚠️  orjson not found, using standard json (slower). Install with: pip install orjson")

import torch
import numpy as np
from pathlib import Path
from collections import defaultdict
from tqdm import tqdm
from transformers import AutoTokenizer
from code_and_train_ds.t3_cpp.MCTS_LABELED_FINAL_TUE_02_20_PM.Scripts.training.PRM_JUDGE.train_adaptive_prm_equal_weights import AdaptivePRM
import argparse


class CrossModelEvaluator:
    def __init__(self, model, tokenizer, device, max_length=6144):
        self.model = model
        self.tokenizer = tokenizer
        self.device = device
        self.max_length = max_length
        self.model.eval()
    
    def format_trajectory(self, steps):
        """Format trajectory steps as text for PRM input"""
        parts = [f"Task: {steps[0].get('task_id', 'unknown')}\n"]
        
        for step in steps:
            parts.append(f"\nStep {step['step_position']}:")
            parts.append(f"Action: {step['action_type']}")
            
            reasoning = step.get('reasoning_normalized', step.get('reasoning', ''))
            if reasoning:
                parts.append(f"Reasoning: {reasoning[:3000]}")
            
            content = step.get('content', '')
            if content and str(content).strip() != str(reasoning).strip():
                parts.append(f"Content: {str(content)[:2500]}")
            
            obs = step.get('observation', {})
            if isinstance(obs, dict):
                if obs.get('compile_error'):
                    parts.append(f"CompileError: {str(obs['compile_error'])[:800]}")
                
                if obs.get('test_summary'):
                    parts.append(f"Tests: {obs['test_summary']}")
                elif obs.get('tests_pass') is not None:
                    parts.append(f"TestsPass: {obs['tests_pass']}")
            
            error_type = step.get('error_type', 'none')
            if error_type != 'none':
                parts.append(f"ErrorType: {error_type}")
        
        return "\n".join(parts)
    
    def score_trajectory(self, steps):
        """Score a single trajectory with PRM (for backward compatibility)"""
        text = self.format_trajectory(steps)
        
        encoding = self.tokenizer(
            text,
            max_length=self.max_length,
            truncation=True,
            padding=False,
            return_tensors='pt'
        )
        
        input_ids = encoding['input_ids'].to(self.device)
        attention_mask = encoding['attention_mask'].to(self.device)
        
        with torch.inference_mode():  # Faster than no_grad
            score = self.model(input_ids, attention_mask)
        
        return score.item()
    
    def score_pretokenized_batch(self, trajectories, batch_size=16, aggregation='last', _retry_count=0):
        """Score pre-tokenized trajectories (10x faster - no tokenization overhead)
        
        Uses same padding as training: torch.nn.utils.rnn.pad_sequence (RIGHT-padding)
        
        Args:
            trajectories: List of trajectory dicts with either:
                - 'input_ids'/'attention_mask' (full trajectory cache)
                - 'prefixes' (prefix cache with step-level prefixes)
            aggregation: 'last' (use full trajectory) or 'mean'/'min'/'max' (use prefixes)
            _retry_count: Internal counter for OOM retries
        """
        # Check if using prefix cache
        is_prefix_cache = 'prefixes' in trajectories[0] if trajectories else False
        
        if aggregation == 'last':
            # Score full trajectories (last step pooling)
            all_scores = []
            
            for i in range(0, len(trajectories), batch_size):
                batch_trajs = trajectories[i:i + batch_size]
                
                # Extract pre-tokenized tensors
                if is_prefix_cache:
                    # Use last prefix (complete trajectory)
                    # Data already in tensor format from cache
                    input_ids_list = [traj['prefixes'][-1]['input_ids'] for traj in batch_trajs]
                    attention_mask_list = [traj['prefixes'][-1]['attention_mask'] for traj in batch_trajs]
                else:
                    # Data already in tensor format from cache
                    input_ids_list = [traj['input_ids'] for traj in batch_trajs]
                    attention_mask_list = [traj['attention_mask'] for traj in batch_trajs]
                
                # Use same padding as training: pad_sequence (RIGHT-padding)
                input_ids = torch.nn.utils.rnn.pad_sequence(
                    input_ids_list,
                    batch_first=True,
                    padding_value=self.tokenizer.pad_token_id
                )
                attention_mask = torch.nn.utils.rnn.pad_sequence(
                    attention_mask_list,
                    batch_first=True,
                    padding_value=0
                )
                
                input_ids = input_ids.to(self.device)
                attention_mask = attention_mask.to(self.device)
                
                with torch.inference_mode():
                    scores = self.model(input_ids, attention_mask)
                
                if scores.dim() == 0:
                    all_scores.append(scores.item())
                else:
                    all_scores.extend(scores.cpu().tolist())
            
            return all_scores
        
        else:
            # OPTIMIZED PER-TRAJECTORY BATCHING
            # Scores prefixes of a SINGLE trajectory at a time to prevent extreme padding explosion.
            if not is_prefix_cache:
                raise ValueError(f"Aggregation '{aggregation}' requires prefix cache, but full trajectory cache provided")
            
            all_traj_scores = []
            
            # Since prefixes in a single trajectory are similar in length, padding overhead is minimal.
            # We can use a moderate batch size here.
            prefix_batch_size = min(32, batch_size)
            
            for traj in trajectories:
                prefixes = traj['prefixes']
                prefix_scores = []
                
                for i in range(0, len(prefixes), prefix_batch_size):
                    batch_prefixes = prefixes[i:i + prefix_batch_size]
                    
                    input_ids_list = [p['input_ids'] for p in batch_prefixes]
                    attention_mask_list = [p['attention_mask'] for p in batch_prefixes]
                    
                    input_ids = torch.nn.utils.rnn.pad_sequence(input_ids_list, batch_first=True, padding_value=self.tokenizer.pad_token_id).to(self.device)
                    attention_mask = torch.nn.utils.rnn.pad_sequence(attention_mask_list, batch_first=True, padding_value=0).to(self.device)
                    
                    with torch.inference_mode():
                        scores = self.model(input_ids, attention_mask)
                    prefix_scores.extend(scores.cpu().tolist() if scores.dim() > 0 else [scores.item()])
                
                # Aggregate scores for this trajectory
                if aggregation == 'mean':
                    traj_score = np.mean(prefix_scores)
                elif aggregation == 'min':
                    traj_score = np.min(prefix_scores)
                elif aggregation == 'max':
                    traj_score = np.max(prefix_scores)
                elif aggregation == 'final_weighted':
                    weights = np.exp(np.linspace(0, 1, len(prefix_scores)))
                    weights = weights / weights.sum()
                    traj_score = np.average(prefix_scores, weights=weights)
                else:
                    raise ValueError(f"Unknown aggregation: {aggregation}")
                
                all_traj_scores.append(float(traj_score))
            
            return all_traj_scores
    
    def score_trajectories_batch(self, list_of_steps_lists, batch_size=16, aggregation='last'):
        """Score multiple trajectories in batches with different aggregation strategies"""
        if aggregation == 'last':
            # Original behavior: score complete trajectory, model returns last step score
            all_scores = []
            for i in range(0, len(list_of_steps_lists), batch_size):
                batch_steps = list_of_steps_lists[i:i + batch_size]
                texts = [self.format_trajectory(steps) for steps in batch_steps]
                
                encoding = self.tokenizer(
                    texts,
                    max_length=self.max_length,
                    truncation=True,
                    padding=True,
                    return_tensors='pt'
                )
                
                input_ids = encoding['input_ids'].to(self.device)
                attention_mask = encoding['attention_mask'].to(self.device)
                
                with torch.inference_mode():
                    scores = self.model(input_ids, attention_mask)
                
                if scores.dim() == 0:
                    all_scores.append(scores.item())
                else:
                    all_scores.extend(scores.cpu().tolist() if scores.dim() > 0 else [scores.item()])
            
            return all_scores
        
        else:
            # FAST BATCHED AGGREGATION: Score all prefixes globally, then aggregate per trajectory
            # Build all prefixes with trajectory mapping
            all_prefixes = []
            traj_prefix_counts = []  # Track how many prefixes per trajectory
            
            for traj_steps in list_of_steps_lists:
                num_steps = len(traj_steps)
                traj_prefix_counts.append(num_steps)
                
                # Add all prefixes for this trajectory
                for step_idx in range(num_steps):
                    all_prefixes.append(traj_steps[:step_idx + 1])
            
            # Score all prefixes in batches
            all_prefix_scores = []
            for i in range(0, len(all_prefixes), batch_size):
                batch_prefixes = all_prefixes[i:i + batch_size]
                texts = [self.format_trajectory(prefix) for prefix in batch_prefixes]
                
                encoding = self.tokenizer(
                    texts,
                    max_length=self.max_length,
                    truncation=True,
                    padding=True,
                    return_tensors='pt'
                )
                
                input_ids = encoding['input_ids'].to(self.device)
                attention_mask = encoding['attention_mask'].to(self.device)
                
                with torch.inference_mode():
                    scores = self.model(input_ids, attention_mask)
                
                if scores.dim() == 0:
                    all_prefix_scores.append(scores.item())
                else:
                    all_prefix_scores.extend(scores.cpu().tolist())
            
            # Aggregate scores per trajectory
            all_traj_scores = []
            score_idx = 0
            
            for num_prefixes in traj_prefix_counts:
                step_scores = all_prefix_scores[score_idx:score_idx + num_prefixes]
                score_idx += num_prefixes
                
                # Aggregate step scores for this trajectory
                if aggregation == 'mean':
                    traj_score = np.mean(step_scores)
                elif aggregation == 'min':
                    traj_score = np.min(step_scores)
                elif aggregation == 'max':
                    traj_score = np.max(step_scores)
                elif aggregation == 'final_weighted':
                    weights = np.exp(np.linspace(0, 1, len(step_scores)))
                    weights = weights / weights.sum()
                    traj_score = np.average(step_scores, weights=weights)
                else:
                    raise ValueError(f"Unknown aggregation: {aggregation}")
                
                all_traj_scores.append(float(traj_score))
            
            return all_traj_scores


def _default_trajectory():
    """Helper function for defaultdict (pickle-compatible)"""
    return {
        'model': None,
        'task_id': None,
        'steps': [],
        'final_outcome': None,
        'repo': None
    }

def _default_model_list():
    """Helper function for nested defaultdict (pickle-compatible)"""
    return defaultdict(list)

def _default_model_stats():
    """Helper function for defaultdict (pickle-compatible)"""
    return {'correct': 0, 'total': 0, 'tasks': []}

def load_test_trajectories(test_file, max_samples=None):
    """Load and group test trajectories by task and model"""
    file_size_gb = Path(test_file).stat().st_size / 1e9
    print(f"Loading test data from {test_file}...")
    print(f"File size: {file_size_gb:.2f} GB (~239k steps)")
    print(f"Using {'orjson (fast)' if USE_ORJSON else 'standard json (slower)'}")
    print("⏳ This will take {'~1 minute' if USE_ORJSON else '1-2 minutes'}...")
    
    trajectories = defaultdict(_default_trajectory)
    
    if USE_ORJSON:
        # orjson requires binary mode and returns bytes
        with open(test_file, 'rb') as f:
            for line in tqdm(f, desc="Loading steps"):
                try:
                    step = orjson.loads(line)
                    traj_id = step['trajectory_id']
                    
                    trajectories[traj_id]['model'] = step['model']
                    trajectories[traj_id]['task_id'] = step['task_id']
                    trajectories[traj_id]['repo'] = step.get('repo', 'unknown')
                    trajectories[traj_id]['steps'].append(step)
                except Exception as e:
                    pass
    else:
        # Standard json with text mode
        with open(test_file, 'r') as f:
            for line in tqdm(f, desc="Loading steps"):
                try:
                    step = json.loads(line)
                    traj_id = step['trajectory_id']
                    
                    trajectories[traj_id]['model'] = step['model']
                    trajectories[traj_id]['task_id'] = step['task_id']
                    trajectories[traj_id]['repo'] = step.get('repo', 'unknown')
                    trajectories[traj_id]['steps'].append(step)
                except Exception as e:
                    pass
    
    # Extract outcome from LAST step with test results (after sorting)
    print("Extracting outcomes from last step with test results...")
    for traj_id, traj in trajectories.items():
        traj['steps'].sort(key=lambda x: x['step_position'])
        
        # Iterate through steps in order, keep updating outcome
        # The LAST step with an outcome wins
        for step in traj['steps']:
            obs = step.get('observation', {})
            if isinstance(obs, dict) and obs.get('tests_pass') is not None:
                traj['final_outcome'] = obs.get('tests_pass')
    
    # Group by task and model
    task_model_trajs = defaultdict(_default_model_list)
    
    for traj_id, traj in trajectories.items():
        task_id = traj['task_id']
        model = traj['model']
        
        # Sort steps by position
        traj['steps'].sort(key=lambda x: x['step_position'])
        
        task_model_trajs[task_id][model].append({
            'traj_id': traj_id,
            'steps': traj['steps'],
            'outcome': traj['final_outcome'],
            'repo': traj['repo']
        })
        
        if max_samples and len(task_model_trajs) >= max_samples:
            break
    
    print(f"Loaded {len(trajectories):,} trajectories")
    print(f"Tasks: {len(task_model_trajs)}")
    print(f"Multi-model tasks: {sum(1 for t in task_model_trajs.values() if len(t) > 1)}")
    
    return task_model_trajs


def load_test_trajectories_from_cache(cache_file, max_samples=None):
    """Load pre-tokenized test trajectories from Arrow cache (10x faster)
    
    Supports two cache formats:
    1. Full trajectory cache (one entry per trajectory) - for 'last' aggregation
    2. Prefix cache (one entry per step prefix) - for 'mean'/'min'/'max' aggregations
    """
    print(f"Loading pre-tokenized cache from {cache_file}...")
    import time
    start = time.time()
    
    from datasets import Dataset as HFDataset
    hf_dataset = HFDataset.load_from_disk(cache_file)
    
    # Detect cache format by checking for 'step_position' field
    is_prefix_cache = 'step_position' in hf_dataset.column_names
    cache_type = "prefix" if is_prefix_cache else "full trajectory"
    print(f"Cache format: {cache_type}")
    
    # Enable zero-copy PyTorch tensor format (avoids Python list conversion)
    hf_dataset = hf_dataset.with_format("torch")
    print(f"✅ Zero-copy tensor format enabled (Arrow → PyTorch direct)")
    
    if max_samples and not is_prefix_cache and len(hf_dataset) > max_samples:
        print(f"Limiting to {max_samples} trajectories for testing")
        hf_dataset = hf_dataset.select(range(max_samples))
    
    load_time = time.time() - start
    print(f"✅ Loaded {len(hf_dataset):,} pre-tokenized {'prefixes' if is_prefix_cache else 'trajectories'} in {load_time:.1f}s")
    
    # Group by task and model
    task_model_trajs = defaultdict(_default_model_list)
    
    if is_prefix_cache:
        # Prefix cache: Group prefixes by trajectory using Pandas (fast)
        print("Extracting metadata to Pandas for fast grouping...")
        df = hf_dataset.select_columns(['step_position', 'task_id', 'model', 'outcome', 'trajectory_id', 'num_steps', 'repo']).to_pandas()
        
        # Get unique trajectories (one row per trajectory)
        trajectory_info = df.groupby('trajectory_id').agg({
            'task_id': 'first',
            'model': 'first',
            'outcome': 'first',
            'num_steps': 'first',
            'repo': 'first'
        }).reset_index()
        
        print(f"Found {len(trajectory_info):,} unique trajectories")
        
        # Print trajectory statistics
        outcome_counts = trajectory_info['outcome'].value_counts()
        total_trajs = len(trajectory_info)
        pass_trajs = outcome_counts.get(True, 0)
        fail_trajs = outcome_counts.get(False, 0)
        none_trajs = trajectory_info['outcome'].isna().sum()
        print(f"  Pass: {pass_trajs:,} ({pass_trajs/total_trajs*100:.1f}%)")
        print(f"  Fail: {fail_trajs:,} ({fail_trajs/total_trajs*100:.1f}%)")
        print(f"  None: {none_trajs:,} ({none_trajs/total_trajs*100:.1f}%)")
        
        # Filter to multi-model tasks if max_samples specified
        if max_samples is not None:
            print(f"\n🎯 Selecting {max_samples} multi-model tasks for testing...")
            task_model_counts = trajectory_info.groupby('task_id')['model'].nunique()
            multi_model_tasks = task_model_counts[task_model_counts > 1].sort_values(ascending=False)
            
            if len(multi_model_tasks) >= max_samples:
                selected_tasks = multi_model_tasks.head(max_samples).index.tolist()
                print(f"Selected {max_samples} tasks with {multi_model_tasks.head(max_samples).tolist()} models each")
            else:
                selected_tasks = multi_model_tasks.index.tolist()
                print(f"⚠️  Only {len(multi_model_tasks)} multi-model tasks available, using all")
            
            trajectory_info = trajectory_info[trajectory_info['task_id'].isin(selected_tasks)]
            print(f"Filtered to {len(trajectory_info)} trajectories across {len(selected_tasks)} tasks")
        
        # Build prefix mapping
        print("Building prefix mapping...")
        traj_prefixes = defaultdict(list)
        for i in range(len(hf_dataset)):
            item = hf_dataset[i]
            traj_id = item['trajectory_id']
            if max_samples is None or traj_id in trajectory_info['trajectory_id'].values:
                traj_prefixes[traj_id].append({
                    'step_position': item['step_position'],
                    'input_ids': item['input_ids'],
                    'attention_mask': item['attention_mask'],
                })
        
        # Group trajectories by task/model
        print("Grouping trajectories by task/model...")
        for _, row in trajectory_info.iterrows():
            task_id = row['task_id']
            model = row['model']
            traj_id = row['trajectory_id']
            
            prefixes = sorted(traj_prefixes[traj_id], key=lambda x: x['step_position'])
            
            task_model_trajs[task_id][model].append({
                'traj_id': traj_id,
                'prefixes': prefixes,
                'outcome': row['outcome'],
                'repo': row['repo'],
                'num_steps': row['num_steps'],
                'is_prefix_cache': True
            })
    else:
        # Full trajectory cache: One entry per trajectory
        if max_samples is not None:
            print(f"\n🎯 Selecting {max_samples} multi-model tasks for testing...")
            df = hf_dataset.select_columns(['task_id', 'model']).to_pandas()
            task_model_counts = df.groupby('task_id')['model'].nunique()
            multi_model_tasks = task_model_counts[task_model_counts > 1].sort_values(ascending=False)
            
            if len(multi_model_tasks) >= max_samples:
                selected_tasks = multi_model_tasks.head(max_samples).index.tolist()
                print(f"Selected {max_samples} tasks with {multi_model_tasks.head(max_samples).tolist()} models each")
            else:
                selected_tasks = multi_model_tasks.index.tolist()
                print(f"⚠️  Only {len(multi_model_tasks)} multi-model tasks available, using all")
            
            print(f"Filtering dataset to selected tasks...")
        
        for i in range(len(hf_dataset)):
            item = hf_dataset[i]
            task_id = item['task_id']
            
            if max_samples is not None and task_id not in selected_tasks:
                continue
            
            model = item['model']
            
            task_model_trajs[task_id][model].append({
                'traj_id': item['trajectory_id'],
                'input_ids': item['input_ids'],
                'attention_mask': item['attention_mask'],
                'outcome': item['outcome'],
                'repo': item['repo'],
                'num_steps': item['num_steps'],
                'is_pretokenized': True
            })
    
    print(f"Tasks: {len(task_model_trajs)}")
    print(f"Multi-model tasks: {sum(1 for t in task_model_trajs.values() if len(t) > 1)}")
    
    return task_model_trajs, is_prefix_cache


def _default_model_stats():
    """Helper function for defaultdict (pickle-compatible)"""
    return {'correct': 0, 'total': 0, 'tasks': []}

def evaluate_cross_model_pooling(task_model_trajs, evaluator, batch_size=16, aggregation='last', use_pretokenized=False, is_prefix_cache=False):
    """Evaluate single-model vs cross-model PRM selection with batch inference
    
    Uses trajectory-aware batching (like training) to avoid OOM on large tasks
    
    Args:
        aggregation: How to aggregate step scores ('last', 'mean', 'min', 'max', 'final_weighted')
        use_pretokenized: If True, trajectories have pre-tokenized input_ids/attention_mask
        is_prefix_cache: If True, cache contains step-level prefixes (supports all aggregations)
    """
    
    results = {
        'single_model': defaultdict(_default_model_stats),
        'cross_model': {'correct': 0, 'total': 0, 'tasks': []},
        'oracle': {'correct': 0, 'total': 0},
        'per_task_details': []
    }
    
    print(f"\nEvaluating cross-model pooling with batch_size={batch_size}...")
    if use_pretokenized:
        print("⚡ Using pre-tokenized cache (10x faster)")
    print("🔧 Using trajectory-aware batching (like training) to prevent OOM")
    
    for task_id in tqdm(sorted(task_model_trajs.keys()), desc="Tasks"):
        models_for_task = task_model_trajs[task_id]
        
        # Skip single-model tasks
        if len(models_for_task) < 2:
            continue
        
        # Oracle: best outcome across all models
        oracle_success = any(
            any(t['outcome'] is True for t in trajs)
            for trajs in models_for_task.values()
        )
        
        if oracle_success:
            results['oracle']['correct'] += 1
        results['oracle']['total'] += 1
        
        # Single-model: PRM selects best trajectory per model
        single_model_best = False
        
        for model, trajs in models_for_task.items():
            # Collect all completed trajectories for this model
            completed_trajs = [traj for traj in trajs if traj['outcome'] is not None]
            
            if completed_trajs:
                try:
                    # Score trajectories with standard batching (no chunking needed)
                    if use_pretokenized:
                        scores = evaluator.score_pretokenized_batch(completed_trajs, batch_size=batch_size, aggregation=aggregation)
                    else:
                        steps_list = [traj['steps'] for traj in completed_trajs]
                        scores = evaluator.score_trajectories_batch(steps_list, batch_size=batch_size, aggregation=aggregation)
                    
                    # Find best scoring trajectory
                    scored_trajs = [(score, traj['outcome'], traj['traj_id']) 
                                   for score, traj in zip(scores, completed_trajs)]
                    
                    best_score, best_outcome, best_traj_id = max(scored_trajs, key=lambda x: x[0])
                    
                    if best_outcome is True:
                        results['single_model'][model]['correct'] += 1
                        single_model_best = True
                        results['single_model'][model]['tasks'].append(task_id)
                    
                    results['single_model'][model]['total'] += 1
                except Exception as e:
                    print(f"Error scoring model {model} trajectories: {e}")
                    import traceback
                    traceback.print_exc()
                    continue
        
        # Cross-model: PRM selects best trajectory across all models
        all_completed_trajs = []
        for model, trajs in models_for_task.items():
            for traj in trajs:
                if traj['outcome'] is not None:
                    all_completed_trajs.append((model, traj))
        
        cross_model_success = False
        selected_model = None
        
        if all_completed_trajs:
            try:
                # Score all trajectories across all models
                trajs_only = [traj for _, traj in all_completed_trajs]
                if use_pretokenized:
                    scores = evaluator.score_pretokenized_batch(trajs_only, batch_size=batch_size, aggregation=aggregation)
                else:
                    steps_list = [traj['steps'] for _, traj in all_completed_trajs]
                    scores = evaluator.score_trajectories_batch(steps_list, batch_size=batch_size, aggregation=aggregation)
                
                # Find best scoring trajectory globally
                scored_trajs = [(score, traj['outcome'], model, traj['traj_id']) 
                               for score, (model, traj) in zip(scores, all_completed_trajs)]
                
                best_score, best_outcome, best_model, best_traj_id = max(scored_trajs, key=lambda x: x[0])
                selected_model = best_model
                
                if best_outcome is True:
                    results['cross_model']['correct'] += 1
                    cross_model_success = True
                    results['cross_model']['tasks'].append(task_id)
                
                results['cross_model']['total'] += 1
                
                # Store per-task details
                results['per_task_details'].append({
                    'task_id': task_id,
                    'num_models': len(models_for_task),
                    'oracle': oracle_success,
                    'single_model_best': single_model_best,
                    'cross_model': cross_model_success,
                    'selected_model': selected_model,
                    'repo': all_completed_trajs[0][1]['repo'] if all_completed_trajs else 'unknown'
                })
            except Exception as e:
                print(f"Error in cross-model scoring for task {task_id}: {e}")
                import traceback
                traceback.print_exc()
                continue
    
    return results


def print_results(results):
    """Print evaluation results"""
    print("\n" + "="*80)
    print("CROSS-MODEL POOLING EVALUATION RESULTS")
    print("="*80)
    
    # Oracle
    oracle_acc = results['oracle']['correct'] / results['oracle']['total'] if results['oracle']['total'] > 0 else 0
    print(f"\n📊 ORACLE (Upper Bound):")
    print(f"   Pass@1: {oracle_acc:.4f} ({results['oracle']['correct']}/{results['oracle']['total']})")
    
    # Single-model (best)
    print(f"\n📊 SINGLE-MODEL PRM SELECTION:")
    best_single_model = None
    best_single_acc = 0
    
    for model, stats in sorted(results['single_model'].items(), key=lambda x: x[1]['correct']/x[1]['total'] if x[1]['total'] > 0 else 0, reverse=True):
        if stats['total'] > 0:
            acc = stats['correct'] / stats['total']
            model_short = model.split('/')[-1] if '/' in model else model
            print(f"   {model_short:<40}: {acc:.4f} ({stats['correct']}/{stats['total']})")
            if acc > best_single_acc:
                best_single_acc = acc
                best_single_model = model
    
    print(f"\n   Best single-model: {best_single_model.split('/')[-1] if best_single_model else 'N/A'} ({best_single_acc:.4f})")
    
    # Cross-model
    cross_acc = results['cross_model']['correct'] / results['cross_model']['total'] if results['cross_model']['total'] > 0 else 0
    print(f"\n📊 CROSS-MODEL PRM SELECTION:")
    print(f"   Pass@1: {cross_acc:.4f} ({results['cross_model']['correct']}/{results['cross_model']['total']})")
    
    # Comparison
    print(f"\n" + "="*80)
    print("COMPARISON & GAINS")
    print("="*80)
    print(f"Best single-model:  {best_single_acc:.4f}")
    print(f"Cross-model:        {cross_acc:.4f}")
    print(f"Oracle:             {oracle_acc:.4f}")
    print()
    
    if best_single_acc > 0:
        gain_abs = cross_acc - best_single_acc
        gain_pct = (cross_acc / best_single_acc - 1) * 100
        print(f"Gain over single:   {gain_abs:+.4f} ({gain_pct:+.1f}%)")
        
        if oracle_acc > best_single_acc:
            gap_closed = (cross_acc - best_single_acc) / (oracle_acc - best_single_acc) * 100
            print(f"Oracle gap closed:  {gap_closed:.1f}%")
    
    print("="*80)
    
    # Model selection distribution
    print(f"\n📊 CROSS-MODEL SELECTION DISTRIBUTION:")
    model_selections = defaultdict(int)
    for detail in results['per_task_details']:
        if detail['selected_model']:
            model_selections[detail['selected_model']] += 1
    
    for model, count in sorted(model_selections.items(), key=lambda x: x[1], reverse=True):
        model_short = model.split('/')[-1] if '/' in model else model
        pct = count / len(results['per_task_details']) * 100 if results['per_task_details'] else 0
        print(f"   {model_short:<40}: {count:>4} ({pct:>5.1f}%)")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--checkpoint', required=True, help='Path to trained PRM checkpoint or "baseline" for untrained model')
    parser.add_argument('--test_file', required=False, default=None, help='Path to test.jsonl (optional if using --test_cache)')
    parser.add_argument('--test_cache', type=str, default=None, help='Path to pre-tokenized test cache (Arrow format)')
    parser.add_argument('--output_dir', default='results/cross_model_pooling', help='Output directory')
    parser.add_argument('--batch_size', type=int, default=128, help='Batch size for inference (128 optimized for A6000 with plenty of VRAM)')
    parser.add_argument('--max_samples', type=int, default=None, help='Max tasks to evaluate (for testing)')
    parser.add_argument('--base_model', type=str, default='Qwen/Qwen2.5-Coder-1.5B')
    parser.add_argument('--aggregation', type=str, default='last', 
                       choices=['last', 'mean', 'min', 'max', 'final_weighted'],
                       help='How to aggregate step scores: last (default), mean, min, max, final_weighted')
    parser.add_argument('--multi_gpu', action='store_true', help='Use multi-GPU with DDP (like training)')
    
    args = parser.parse_args()
    
    # Validate arguments
    if not args.test_cache and not args.test_file:
        print("❌ Error: Either --test_file or --test_cache must be provided")
        return
    
    # DDP: Initialize process group if multi_gpu enabled
    import torch.distributed as dist
    import os as os_module
    is_distributed = args.multi_gpu and dist.is_available() and "RANK" in os.environ
    
    if is_distributed:
        from datetime import timedelta
        dist.init_process_group(backend="nccl", timeout=timedelta(hours=3))
        rank = dist.get_rank()
        world_size = dist.get_world_size()
        torch.cuda.set_device(int(os.environ["LOCAL_RANK"]))
        device = torch.device(f'cuda:{torch.cuda.current_device()}')
        print(f"Rank {rank}/{world_size}: Initialized DDP with 3-hour timeout")
    else:
        rank = 0
        world_size = 1
        device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    
    if rank == 0:
        print(f"Using device: {device}")
        print(f"Multi-GPU: {is_distributed} (world_size={world_size})")
    
    # Load tokenizer
    if rank == 0:
        print(f"\nLoading tokenizer from {args.base_model}...")
    tokenizer = AutoTokenizer.from_pretrained(args.base_model)
    tokenizer.pad_token = tokenizer.eos_token
    
    # Load model
    if rank == 0:
        print(f"\n{'='*80}")
        if args.checkpoint.lower() == 'baseline':
            print(f"LOADING BASELINE (UNTRAINED) MODEL")
            print(f"{'='*80}")
            print(f"Base model: {args.base_model}")
            print(f"Status: Random initialization (no training)")
        else:
            print(f"LOADING TRAINED PRM CHECKPOINT")
            print(f"{'='*80}")
            print(f"Checkpoint: {args.checkpoint}")
            print(f"Base model: {args.base_model}")
    
    if args.checkpoint.lower() == 'baseline':
        model = AdaptivePRM(args.base_model, use_lora=True, freeze_encoder=False)
        if rank == 0:
            print(f"✅ Baseline model created (untrained weights)")
    else:
        # Check if checkpoint exists
        checkpoint_path = Path(args.checkpoint)
        if not checkpoint_path.exists():
            if rank == 0:
                print(f"❌ Checkpoint not found: {args.checkpoint}")
            import sys
            sys.exit(1)
        
        if rank == 0:
            print(f"Loading model weights...")
        model = AdaptivePRM(args.base_model, use_lora=True, freeze_encoder=False)
        
        from safetensors.torch import load_file
        state_dict = load_file(f"{args.checkpoint}/model.safetensors")
        model.load_state_dict(state_dict, strict=False)
        
        if rank == 0:
            print(f"✅ Trained model weights loaded successfully")
    
    if rank == 0:
        print(f"{'='*80}\n")
    
    model = model.to(device)
    model = model.to(torch.bfloat16)
    model.eval()
    
    # DDP: Wrap model if distributed
    if is_distributed:
        from torch.nn.parallel import DistributedDataParallel as DDP
        model = DDP(model, device_ids=[torch.cuda.current_device()])
        if rank == 0:
            print(f"✅ Model wrapped with DDP")
    
    if rank == 0:
        print("⚠️  Skipping torch.compile() due to environment compatibility")
        print("   Using eager mode with batch inference + inference_mode optimizations")
        print(f"{'='*80}\n")
    
    # Create evaluator
    evaluator = CrossModelEvaluator(model, tokenizer, device)
    
    # Load test data (with optional pre-tokenized cache)
    is_prefix_cache = False
    if args.test_cache and Path(args.test_cache).exists():
        if rank == 0:
            print(f"\n{'='*80}")
            print("LOADING PRE-TOKENIZED TEST CACHE")
            print(f"{'='*80}")
            print(f"Cache: {args.test_cache}")
        task_model_trajs, is_prefix_cache = load_test_trajectories_from_cache(args.test_cache, args.max_samples)
    elif args.test_file:
        if args.test_cache and rank == 0:
            print(f"⚠️  Cache not found: {args.test_cache}")
            print("   Falling back to on-the-fly tokenization (slower)")
        if rank == 0:
            task_model_trajs = load_test_trajectories(args.test_file, args.max_samples)
        else:
            task_model_trajs = {}  # Non-rank-0 processes don't need data
    else:
        if rank == 0:
            print("❌ Error: No valid test data source found")
        return
    
    # DDP: Shard tasks across GPUs (like training shards trajectories)
    if is_distributed:
        all_tasks = sorted(task_model_trajs.keys())
        rank_tasks = all_tasks[rank::world_size]
        rank_task_model_trajs = {task: task_model_trajs[task] for task in rank_tasks}
        if rank == 0:
            print(f"\n📊 DDP Task Sharding:")
        print(f"Rank {rank}: {len(rank_tasks)} tasks ({len(rank_tasks)/len(all_tasks)*100:.1f}% of {len(all_tasks)} total)")
        task_model_trajs = rank_task_model_trajs
    
    # Evaluate with batch inference
    if rank == 0:
        print(f"\n📊 Using aggregation strategy: {args.aggregation}")
        if args.aggregation != 'last':
            print(f"   ⚠️  Non-'last' aggregation will score each step prefix (slower but more accurate)")
        print()
    
    use_pretokenized = args.test_cache and Path(args.test_cache).exists()
    
    # CRITICAL SAFEGUARD: Full trajectory cache only supports 'last' aggregation
    if use_pretokenized and not is_prefix_cache and args.aggregation != 'last':
        if rank == 0:
            print(f"\n{'='*80}")
            print("❌ ERROR: Incompatible Configuration")
            print(f"{'='*80}")
            print(f"You requested aggregation='{args.aggregation}' with FULL trajectory cache.")
            print(f"")
            print(f"Full trajectory cache stores complete trajectories and cannot be split into step prefixes.")
            print(f"Only aggregation='last' is supported with full trajectory cache.")
            print(f"")
            print(f"Solutions:")
            print(f"  1. Use aggregation='last' (recommended, fastest):")
            print(f"     python eval_cross_model_pooling.py --test_cache {args.test_cache} --aggregation last")
            print(f"  2. Use PREFIX cache (supports all aggregations):")
            print(f"     python eval_cross_model_pooling.py --test_cache <prefix_cache_path> --aggregation {args.aggregation}")
            print(f"  3. Remove --test_cache to use raw JSONL (slower, supports all aggregations):")
            print(f"     python eval_cross_model_pooling.py --test_file {args.test_file} --aggregation {args.aggregation}")
            print(f"{'='*80}")
        return
    
    results = evaluate_cross_model_pooling(
        task_model_trajs, 
        evaluator, 
        batch_size=args.batch_size, 
        aggregation=args.aggregation,
        use_pretokenized=use_pretokenized,
        is_prefix_cache=is_prefix_cache
    )
    
    # DDP: Gather results from all ranks
    if is_distributed:
        if rank == 0:
            print(f"\n📊 Gathering results from all ranks...")
        
        # Gather results using all_gather_object
        gathered_results = [None] * world_size
        dist.all_gather_object(gathered_results, results)
        
        if rank == 0:
            # Merge results from all ranks
            merged_results = {
                'single_model': defaultdict(_default_model_stats),
                'cross_model': {'correct': 0, 'total': 0, 'tasks': []},
                'oracle': {'correct': 0, 'total': 0},
                'per_task_details': []
            }
            
            for rank_results in gathered_results:
                # Merge single_model
                for model, stats in rank_results['single_model'].items():
                    merged_results['single_model'][model]['correct'] += stats['correct']
                    merged_results['single_model'][model]['total'] += stats['total']
                    merged_results['single_model'][model]['tasks'].extend(stats['tasks'])
                
                # Merge cross_model
                merged_results['cross_model']['correct'] += rank_results['cross_model']['correct']
                merged_results['cross_model']['total'] += rank_results['cross_model']['total']
                merged_results['cross_model']['tasks'].extend(rank_results['cross_model']['tasks'])
                
                # Merge oracle
                merged_results['oracle']['correct'] += rank_results['oracle']['correct']
                merged_results['oracle']['total'] += rank_results['oracle']['total']
                
                # Merge per_task_details
                merged_results['per_task_details'].extend(rank_results['per_task_details'])
            
            results = merged_results
    
    # Print and save results (only rank 0)
    if rank == 0:
        # Print results
        print_results(results)
        
        # Save results
        output_dir = Path(args.output_dir)
        output_dir.mkdir(parents=True, exist_ok=True)
        
        print(f"\n{'='*80}")
        print(f"SAVING RESULTS")
        print(f"{'='*80}")
        print(f"Output directory: {output_dir.absolute()}")
        print(f"Checkpoint used: {args.checkpoint}")
        
        output_file = output_dir / 'cross_model_results.json'
        with open(output_file, 'w') as f:
            # Convert defaultdict to dict for JSON serialization
            results_serializable = {
                'checkpoint': args.checkpoint,
                'base_model': args.base_model,
                'test_file': args.test_file,
                'aggregation': args.aggregation,
                'multi_gpu': is_distributed,
                'world_size': world_size,
                'single_model': {k: dict(v) for k, v in results['single_model'].items()},
                'cross_model': results['cross_model'],
                'oracle': results['oracle'],
                'per_task_details': results['per_task_details']
            }
            json.dump(results_serializable, f, indent=2)
        
        print(f"\n✅ Results saved to: {output_file.absolute()}")
        print(f"{'='*80}")
    
    # Cleanup DDP
    if is_distributed:
        dist.destroy_process_group()


if __name__ == '__main__':
    main()
