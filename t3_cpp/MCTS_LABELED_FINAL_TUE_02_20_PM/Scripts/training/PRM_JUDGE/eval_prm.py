#!/usr/bin/env python3
"""
Flexible PRM Evaluation - Run any combination of experiments
Usage:
  python eval_prm.py --exp trained --checkpoint path/to/model --test_file test.jsonl --output_dir results/trained
  python eval_prm.py --exp untrained --test_file test.jsonl --output_dir results/untrained
  python eval_prm.py --exp ablation --checkpoint path/to/model --test_file test.jsonl --output_dir results/ablation
  python eval_prm.py --exp ablation_untrained --test_file test.jsonl --output_dir results/ablation_untrained
"""

import json
import torch
import numpy as np
from pathlib import Path
from collections import defaultdict
from scipy.stats import spearmanr, kendalltau
from transformers import AutoTokenizer
from train_adaptive_prm import AdaptivePRM, PRMDataset, collate_fn
import argparse
from tqdm import tqdm
from torch.utils.data import Dataset, DataLoader


class PRMEvalDataset(Dataset):
    """Dataset that can compute labels on-the-fly with different configurations"""
    def __init__(self, data_file, tokenizer, max_length=6144, 
                 label_mode='hybrid', alpha=0.4, max_samples=None):
        self.tokenizer = tokenizer
        self.max_length = max_length
        self.label_mode = label_mode
        self.alpha = alpha
        self.samples = []
        
        print(f"Loading {data_file} with label_mode={label_mode}, alpha={alpha}...")
        
        traj_cache = defaultdict(list)
        with open(data_file) as f:
            for line in f:
                step = json.loads(line)
                traj_cache[step['trajectory_id']].append(step)
                if max_samples and len(traj_cache) >= max_samples:
                    break
        
        for traj_id, steps in tqdm(traj_cache.items(), desc="Processing"):
            steps.sort(key=lambda x: x['step_position'])
            
            for step in steps:
                if not step.get('reliable', True):
                    continue
                if not step.get('include_in_training', True):
                    continue
                
                label = self.compute_label(step, len(steps))
                text = self.format_input(steps[:step['step_position']+1])
                
                encoding = tokenizer(
                    text,
                    max_length=self.max_length,
                    truncation=True,
                    padding=False
                )
                
                self.samples.append({
                    'trajectory_id': traj_id,
                    'input_ids': encoding['input_ids'],
                    'attention_mask': encoding['attention_mask'],
                    'label': label,
                    'weight': 1.0,  # Default weight for evaluation
                    'mc_label': step.get('mc_label_soft', 0.5),
                    'judge_score': step.get('judge_overall_score', 0.5),
                })
        
        print(f"Loaded {len(self.samples)} samples from {len(traj_cache)} trajectories")
    
    def compute_label(self, step, traj_len):
        mc_adv = step.get('mc_advantage', 0.0) or 0.0
        mc_label = 0.5 + 0.5 * np.tanh(mc_adv / 0.1)
        judge_score = step.get('judge_overall_score', 0.5) or 0.5
        
        if self.label_mode == 'mc_only':
            return float(mc_label)
        elif self.label_mode == 'judge_only':
            return float(judge_score)
        elif self.label_mode == 'hybrid':
            return float(self.alpha * mc_label + (1 - self.alpha) * judge_score)
        else:
            return float(self.alpha * mc_label + (1 - self.alpha) * judge_score)
    
    def format_input(self, trajectory_prefix):
        parts = [f"Task: {trajectory_prefix[0].get('task_id', 'unknown')}\n"]
        for step in trajectory_prefix:
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
    
    def __len__(self):
        return len(self.samples)
    
    def __getitem__(self, idx):
        return self.samples[idx]


def evaluate_model(model, dataset, tokenizer, device, batch_size=8):
    model.eval()
    dataloader = DataLoader(
        dataset, 
        batch_size=batch_size, 
        collate_fn=collate_fn(tokenizer),
        shuffle=False
    )
    
    traj_predictions = defaultdict(list)
    traj_labels = defaultdict(list)
    
    with torch.no_grad():
        for batch in tqdm(dataloader, desc="Evaluating"):
            batch = {k: v.to(device) if isinstance(v, torch.Tensor) else v 
                    for k, v in batch.items()}
            
            traj_ids = batch.pop('trajectory_id')
            labels = batch.pop('labels')
            weights = batch.pop('weights')
            
            preds = model(**batch)
            
            for tid, pred, label in zip(traj_ids, preds.float().cpu().numpy(), labels.float().cpu().numpy()):
                traj_predictions[tid].append(pred)
                traj_labels[tid].append(label)
    
    metrics = {
        'ranking_accuracy': float(compute_ranking_accuracy(traj_predictions, traj_labels)),
        'spearman': float(compute_spearman(traj_predictions, traj_labels)),
        'kendall_tau': float(compute_kendall_tau(traj_predictions, traj_labels)),
        'mse': float(compute_mse(traj_predictions, traj_labels)),
        'mae': float(compute_mae(traj_predictions, traj_labels)),
    }
    
    return metrics, traj_predictions, traj_labels


def compute_ranking_accuracy(predictions, labels):
    correct = 0
    total = 0
    for traj_id in predictions:
        preds = predictions[traj_id]
        labs = labels[traj_id]
        if len(preds) < 2:
            continue
        for i in range(len(preds)):
            for j in range(i + 1, len(preds)):
                if labs[i] != labs[j]:
                    if (labs[i] > labs[j]) == (preds[i] > preds[j]):
                        correct += 1
                    total += 1
    return correct / total if total > 0 else 0.0


def compute_spearman(predictions, labels):
    correlations = []
    for traj_id in predictions:
        preds = predictions[traj_id]
        labs = labels[traj_id]
        if len(preds) < 2 or len(set(labs)) < 2:
            continue
        rho, _ = spearmanr(preds, labs)
        if not np.isnan(rho):
            correlations.append(rho)
    return np.mean(correlations) if correlations else 0.0


def compute_kendall_tau(predictions, labels):
    correlations = []
    for traj_id in predictions:
        preds = predictions[traj_id]
        labs = labels[traj_id]
        if len(preds) < 2 or len(set(labs)) < 2:
            continue
        tau, _ = kendalltau(preds, labs)
        if not np.isnan(tau):
            correlations.append(tau)
    return np.mean(correlations) if correlations else 0.0


def compute_mse(predictions, labels):
    all_preds = []
    all_labels = []
    for traj_id in predictions:
        all_preds.extend(predictions[traj_id])
        all_labels.extend(labels[traj_id])
    return np.mean((np.array(all_preds) - np.array(all_labels)) ** 2)


def compute_mae(predictions, labels):
    all_preds = []
    all_labels = []
    for traj_id in predictions:
        all_preds.extend(predictions[traj_id])
        all_labels.extend(labels[traj_id])
    return np.mean(np.abs(np.array(all_preds) - np.array(all_labels)))


def evaluate_reranking(predictions, labels, k_values=[1, 3, 5, 10]):
    results = {}
    for k in k_values:
        correct = 0
        total = 0
        for traj_id in predictions:
            preds = predictions[traj_id]
            labs = labels[traj_id]
            if len(preds) < k:
                continue
            top_k_indices = np.argsort(preds)[-k:]
            best_label = max([labs[i] for i in top_k_indices])
            if best_label > 0.5:
                correct += 1
            total += 1
        results[f'rerank@{k}'] = float(correct / total) if total > 0 else 0.0
    return results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--exp', type=str, required=True,
                       choices=['trained', 'untrained', 'ablation', 'ablation_untrained'],
                       help='Which experiment to run')
    parser.add_argument('--checkpoint', type=str, default=None,
                       help='Path to trained checkpoint (required for trained/ablation)')
    parser.add_argument('--test_file', type=str, required=True)
    parser.add_argument('--output_dir', type=str, required=True)
    parser.add_argument('--batch_size', type=int, default=8)
    parser.add_argument('--max_samples', type=int, default=None)
    parser.add_argument('--base_model', type=str, default='Qwen/Qwen2.5-Coder-1.5B')
    args = parser.parse_args()
    
    # Validate checkpoint requirement
    if args.exp in ['trained', 'ablation'] and not args.checkpoint:
        raise ValueError(f"--checkpoint required for experiment '{args.exp}'")
    
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"Using device: {device}")
    
    tokenizer = AutoTokenizer.from_pretrained(args.base_model)
    tokenizer.pad_token = tokenizer.eos_token
    
    # Create model
    if args.exp in ['trained', 'ablation']:
        checkpoint_name = args.checkpoint.rstrip('/').split('/')[-1]
        print(f"\n{'='*70}")
        print(f"📦 LOADING CHECKPOINT: {checkpoint_name}")
        print(f"📂 Full path: {args.checkpoint}")
        print(f"{'='*70}")
        model = AdaptivePRM(args.base_model, use_lora=True, freeze_encoder=False)
        from safetensors.torch import load_file
        state_dict = load_file(f"{args.checkpoint}/model.safetensors")
        model.load_state_dict(state_dict, strict=False)
        print(f"✅ Trained weights loaded from {checkpoint_name}")
        print(f"{'='*70}\n")
    else:
        print("Creating untrained model (random initialization)...")
        model = AdaptivePRM(args.base_model, use_lora=True, freeze_encoder=False)
    
    model = model.to(device)
    model = model.to(torch.bfloat16)  # Ensure consistent dtype
    model.eval()
    
    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Run experiment
    if args.exp in ['trained', 'untrained']:
        # Single evaluation with hybrid labels
        print(f"\nRunning {args.exp} evaluation...")
        dataset = PRMEvalDataset(args.test_file, tokenizer, max_length=6144,
                                label_mode='hybrid', alpha=0.4, max_samples=args.max_samples)
        
        metrics, predictions, labels = evaluate_model(model, dataset, tokenizer, device, args.batch_size)
        rerank_metrics = evaluate_reranking(predictions, labels)
        
        print(f"\n📊 {args.exp.upper()} RESULTS:")
        print(f"  Ranking Accuracy: {metrics['ranking_accuracy']:.4f}")
        print(f"  Spearman: {metrics['spearman']:.4f}")
        print(f"  MSE: {metrics['mse']:.6f}")
        
        print(f"\n📊 RERANKING:")
        for k, score in rerank_metrics.items():
            print(f"  {k}: {score:.4f}")
        
        results = {
            'experiment': args.exp,
            'checkpoint': args.checkpoint,
            'metrics': metrics,
            'reranking': rerank_metrics,
        }
        
        output_file = output_dir / f'{args.exp}_results.json'
    
    else:  # ablation or ablation_untrained
        # Multiple evaluations with different label configs
        print(f"\nRunning {args.exp} (label ablation)...")
        
        label_configs = [
            {'name': 'mc_only', 'mode': 'mc_only', 'alpha': 1.0},
            {'name': 'judge_only', 'mode': 'judge_only', 'alpha': 0.0},
            {'name': 'hybrid_0.2', 'mode': 'hybrid', 'alpha': 0.2},
            {'name': 'hybrid_0.4', 'mode': 'hybrid', 'alpha': 0.4},
            {'name': 'hybrid_0.6', 'mode': 'hybrid', 'alpha': 0.6},
        ]
        
        all_results = {}
        for config in label_configs:
            print(f"\n{'─'*70}")
            print(f"Evaluating: {config['name']}")
            print(f"{'─'*70}")
            
            dataset = PRMEvalDataset(args.test_file, tokenizer, max_length=6144,
                                    label_mode=config['mode'], alpha=config['alpha'],
                                    max_samples=args.max_samples)
            
            metrics, _, _ = evaluate_model(model, dataset, tokenizer, device, args.batch_size)
            
            print(f"  Ranking Accuracy: {metrics['ranking_accuracy']:.4f}")
            print(f"  Spearman: {metrics['spearman']:.4f}")
            print(f"  MSE: {metrics['mse']:.6f}")
            
            all_results[config['name']] = metrics
        
        # Print comparison table
        print(f"\n{'='*70}")
        print(f"LABEL ABLATION SUMMARY ({args.exp})")
        print(f"{'='*70}")
        print(f"{'Configuration':<20} {'Rank Acc':<12} {'Spearman':<12} {'MSE':<12}")
        print("─"*70)
        
        for config_name, metrics in all_results.items():
            print(f"{config_name:<20} {metrics['ranking_accuracy']:<12.4f} "
                  f"{metrics['spearman']:<12.4f} {metrics['mse']:<12.6f}")
        
        results = {
            'experiment': args.exp,
            'checkpoint': args.checkpoint,
            'results': all_results,
        }
        
        output_file = output_dir / f'{args.exp}_results.json'
    
    with open(output_file, 'w') as f:
        json.dump(results, f, indent=2)
    
    print(f"\n✅ Results saved to {output_file}")
    print("="*70)


if __name__ == '__main__':
    main()
