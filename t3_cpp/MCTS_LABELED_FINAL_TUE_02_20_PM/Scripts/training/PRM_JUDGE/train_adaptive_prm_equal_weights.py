#!/usr/bin/env python3
"""
train_adaptive_prm.py

Fast discriminative PRM training with SIMPLE labels + STRONG LDCA
- 50% ListNet (ranking) + 50% MSE (calibration) - INCREASED MSE WEIGHT
- SIMPLE 50-50 label blending (no adaptive heuristics)
- STRONG LDCA weighting (alpha_victim=2.0, alpha_lucky=1.5)
- LoRA for efficient fine-tuning (default)
- Multi-GPU with DDP (automatic)

KEY CHANGES FROM ORIGINAL:
1. Removed adaptive label computation (action type, position, consensus adjustments)
2. Simple 50-50 MC/Judge blend for labels
3. Increased LDCA strength (2.0x victim amplification, 1.5x lucky dampening)
4. Increased MSE weight from 30% to 50% (LDCA primarily affects MSE)
5. Wider LDCA weight range [0.25, 3.0] (was [0.5, 2.0])
"""

import json
import os
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
from pathlib import Path
from collections import defaultdict
from typing import Dict, List, Optional
from transformers import (
    AutoTokenizer, 
    AutoModel,
    Trainer,
    TrainingArguments,
)
from torch.utils.data import Dataset, Sampler
from peft import LoraConfig, get_peft_model, TaskType
import argparse
from tqdm import tqdm
from datasets import Dataset as HFDataset
import random
from datetime import timedelta

# Try importing scipy for Spearman correlation
try:
    from scipy.stats import spearmanr
    HAS_SCIPY = True
except ImportError:
    HAS_SCIPY = False
    print("Warning: scipy not available, Spearman correlation will be skipped in evaluation")

# ============================================
# MODEL
# ============================================

class AdaptivePRM(nn.Module):
    """Discriminative scalar PRM with optional LoRA"""
    
    def __init__(self, base_model="Qwen/Qwen2.5-Coder-1.5B", use_lora=True, freeze_encoder=False, use_qlora=False):
        super().__init__()
        print("Loading model with Flash Attention 2...")
        
        # QLoRA: 4-bit quantization for massive memory savings
        if use_qlora:
            print("Using QLoRA (4-bit quantization)...")
            from transformers import BitsAndBytesConfig
            quantization_config = BitsAndBytesConfig(
                load_in_4bit=True,
                bnb_4bit_compute_dtype=torch.bfloat16,
                bnb_4bit_use_double_quant=True,
                bnb_4bit_quant_type="nf4"
            )
            try:
                self.encoder = AutoModel.from_pretrained(
                    base_model,
                    quantization_config=quantization_config,
                    trust_remote_code=True,
                    attn_implementation="flash_attention_2"
                )
                print("✅ QLoRA enabled with Flash Attention 2")
            except Exception as e:
                print(f"⚠️  Flash Attention 2 unavailable, using SDPA with QLoRA")
                self.encoder = AutoModel.from_pretrained(
                    base_model,
                    quantization_config=quantization_config,
                    trust_remote_code=True,
                    attn_implementation="sdpa"
                )
        else:
            try:
                self.encoder = AutoModel.from_pretrained(
                    base_model,
                    dtype=torch.bfloat16,
                    trust_remote_code=True,
                    attn_implementation="flash_attention_2"
                )
                print("✅ Flash Attention 2 enabled")
            except Exception as e:
                print(f"⚠️  Flash Attention 2 unavailable ({e}), falling back to SDPA")
                self.encoder = AutoModel.from_pretrained(
                    base_model,
                    dtype=torch.bfloat16,
                    trust_remote_code=True,
                    attn_implementation="sdpa"
                )
                print("✅ SDPA attention enabled")
        
        # Disable cache for gradient checkpointing compatibility
        if hasattr(self.encoder.config, 'use_cache'):
            self.encoder.config.use_cache = False
        
        # Apply LoRA if requested
        if use_lora and not freeze_encoder:
            print("Applying LoRA to encoder...")
            lora_config = LoraConfig(
                task_type=TaskType.FEATURE_EXTRACTION,
                r=16,
                lora_alpha=32,
                lora_dropout=0.1,
                target_modules=["q_proj", "v_proj", "k_proj", "o_proj"],
                bias="none",
            )
            self.encoder = get_peft_model(self.encoder, lora_config)
            if use_qlora:
                # QLoRA requires special handling for gradient checkpointing
                self.encoder.enable_input_require_grads()
            else:
                self.encoder.enable_input_require_grads()  # Required for gradient checkpointing + LoRA
            self.encoder.print_trainable_parameters()
        elif freeze_encoder:
            print("Freezing encoder (training head only)...")
            for param in self.encoder.parameters():
                param.requires_grad = False
        else:
            print("Full fine-tuning encoder...")
        
        # Get hidden size (handle different config structures)
        # Qwen3.5 uses text_config.hidden_size, older models use config.hidden_size
        if hasattr(self.encoder.config, 'text_config'):
            hidden_size = self.encoder.config.text_config.hidden_size
        else:
            hidden_size = getattr(self.encoder.config, 'hidden_size', 
                                 getattr(self.encoder.config, 'd_model', 
                                        getattr(self.encoder.config, 'n_embd', 4096)))
        
        # Simple reward head
        self.reward_head = nn.Sequential(
            nn.Linear(hidden_size, 256),
            nn.ReLU(),
            nn.Dropout(0.1),
            nn.Linear(256, 1)
        )
    
    def gradient_checkpointing_enable(self, gradient_checkpointing_kwargs=None):
        """Enable gradient checkpointing for the encoder"""
        # Disable cache before enabling gradient checkpointing
        if hasattr(self.encoder.config, 'use_cache'):
            self.encoder.config.use_cache = False
        
        if hasattr(self.encoder, 'gradient_checkpointing_enable'):
            self.encoder.gradient_checkpointing_enable(gradient_checkpointing_kwargs)
    
    def gradient_checkpointing_disable(self):
        """Disable gradient checkpointing for the encoder"""
        if hasattr(self.encoder, 'gradient_checkpointing_disable'):
            self.encoder.gradient_checkpointing_disable()
    
    def forward(self, input_ids, attention_mask):
        outputs = self.encoder(input_ids=input_ids, attention_mask=attention_mask)
        hidden = outputs.last_hidden_state
        
        # Last token pooling
        lengths = (attention_mask.sum(dim=1) - 1).clamp(0, hidden.size(1) - 1)
        pooled = hidden[torch.arange(hidden.size(0)), lengths]
        
        # Raw score (no sigmoid - for ListNet)
        score = self.reward_head(pooled).squeeze(-1)
        return score


# ============================================
# LABEL COMPUTATION
# ============================================

def compute_simple_label(step, traj_len):
    """
    Compute SIMPLE hybrid label with 50-50 blending
    
    NO adaptive heuristics:
    - No action-type adjustment
    - No position adjustment
    - No consensus adjustment
    
    Let LDCA handle disagreement via loss weighting instead of label adjustment.
    This preserves disagreement information for the model to learn from.
    """
    # MC component (use pre-computed advantage)
    mc_adv = step.get('mc_advantage', 0.0)
    if mc_adv is None:
        mc_adv = 0.0
    L_MC = 0.5 + 0.5 * np.tanh(mc_adv / 0.1)
    
    # Judge component (handle None)
    judge_scores = step.get('judge_axis_scores') or {}
    p_t = judge_scores.get('process_quality', 0.5)
    if p_t is None:
        p_t = 0.5
    c_t = judge_scores.get('correctness')
    
    if c_t is not None and step.get('action_type') == 'generate_code':
        L_judge = 0.6 * c_t + 0.4 * p_t
    else:
        L_judge = p_t
    
    # SIMPLE 50-50 blend (no adaptive alpha)
    label = 0.5 * L_MC + 0.5 * L_judge
    label = float(np.clip(label, 0.0, 1.0))
    return label


def compute_adaptive_label(step, traj_len):
    """
    DEPRECATED: Use compute_simple_label() instead.
    Kept for backward compatibility with cached data.
    """
    return compute_simple_label(step, traj_len)


def compute_ldca_weight(step, alpha_victim=2.0, alpha_lucky=1.5):
    """
    LDCA: Luck-Decoupled Credit Assignment (STRONG VERSION)
    
    Asymmetric loss weighting based on disagreement DIRECTION:
    - Victim steps (Judge high, MC low): good process in failed trajectory → AMPLIFY STRONGLY
    - Lucky steps (MC high, Judge low): bad process in successful trajectory → DAMPEN STRONGLY
    
    Args:
        step: Dict containing mc_advantage, judge_overall_score
        alpha_victim: Amplification factor for victim steps (default 2.0, was 1.3)
        alpha_lucky: Dampening factor for lucky steps (default 1.5, was 0.7)
    
    Returns:
        weight: Float in [0.25, 3.0] (wider range than before: [0.5, 2.0])
    
    CHANGES FROM ORIGINAL:
    - Increased alpha_victim: 1.3 → 2.0 (54% stronger amplification)
    - Increased alpha_lucky: 0.7 → 1.5 (114% stronger dampening)
    - Wider weight range: [0.5, 2.0] → [0.25, 3.0]
    - Lower activation threshold: 0.2 → 0.15 (more sensitive)
    """
    # Check reliability first (1.82% unreliable)
    if not step.get('reliable', True):
        return 1.0
    
    # Normalize MC to [0,1]
    mc_adv = step.get('mc_advantage', 0.0)
    if mc_adv is None:
        return 1.0
    mc_score = 0.5 + 0.5 * np.tanh(mc_adv / 0.1)
    
    # Get Judge score (already in [0,1])
    judge_score = step.get('judge_overall_score', 0.5)
    if judge_score is None:
        return 1.0
    
    # Compute "luck" = MC - Judge
    # Positive luck: MC thinks good, Judge thinks bad (lucky step)
    # Negative luck: Judge thinks good, MC thinks bad (victim step)
    luck = mc_score - judge_score
    
    # Asymmetric weighting based on direction (LOWER THRESHOLD: 0.2 → 0.15)
    if luck < -0.15:
        # VICTIM STEP: Judge scored high, but MC scored low
        # This is likely a good step in a failed trajectory
        # AMPLIFY gradient STRONGLY to teach PRM to recognize it as good
        weight = 1.0 + alpha_victim * abs(luck)
    
    elif luck > 0.15:
        # LUCKY STEP: MC scored high, but Judge scored low
        # This is likely a bad step in a successful trajectory
        # DAMPEN gradient STRONGLY to prevent PRM from learning it as good
        weight = 1.0 - alpha_lucky * luck
    
    else:
        # AGREEMENT ZONE: MC and Judge roughly agree
        # Normal weighting
        weight = 1.0
    
    # Bound weight to WIDER range (was [0.5, 2.0], now [0.25, 3.0])
    weight = float(np.clip(weight, 0.25, 3.0))
    
    return weight


# ============================================
# DATASET
# ============================================

def safe_get_float(step, key, default=0.5):
    """Safely get float value, handling None"""
    value = step.get(key)
    return default if value is None else value

class PRMDataset(Dataset):
    def __init__(self, data_file, tokenizer, max_length=2048, max_samples=None, 
                 alpha_victim=2.0, alpha_lucky=1.5, load_full_file=False, tokenize_only=False, use_arrow=False, cache_override=None):
        self.tokenizer = tokenizer
        self.max_length = max_length
        self.alpha_victim = alpha_victim
        self.alpha_lucky = alpha_lucky
        self.samples = []
        self.tokenize_only = tokenize_only
        self.use_arrow = use_arrow
        self.filter_stats = {
            'total_steps': 0,
            'filtered_reliable_false': 0,
            'filtered_mc_reliable_false': 0,
            'filtered_timeout_sandbox': 0,
            'filtered_include_false': 0,
            'kept_steps': 0,
            'trajectories_total': 0,
            'trajectories_with_excluded': 0,
            'trajectories_all_excluded': 0,
        }
        
        # Check for cache override (stratified subset)
        if cache_override and Path(cache_override).exists():
            print(f"Loading stratified cache from {cache_override}...")
            import time
            start = time.time()
            hf_dataset = HFDataset.load_from_disk(cache_override)
            load_time = time.time() - start
            
            # Support max_samples for smoke test
            if max_samples and len(hf_dataset) > max_samples:
                print(f"Smoke test mode: selecting first {max_samples} samples from {len(hf_dataset):,}")
                hf_dataset = hf_dataset.select(range(max_samples))
            
            print(f"Loaded {len(hf_dataset):,} pre-tokenized samples from stratified cache in {load_time:.1f}s")
            
            # Enable PyTorch tensor output for zero-copy dataloading
            hf_dataset.set_format(type="torch")
            
            # Use HF Dataset directly (memory-mapped, zero-copy)
            self.samples = hf_dataset
            
            # LDCA statistics with caching
            if len(self.samples) > 0:
                stats_cache = cache_override.replace('.arrow', '_ldca_stats.json')
                if Path(stats_cache).exists():
                    import json as json_module
                    with open(stats_cache) as f:
                        stats = json_module.load(f)
                    print(f"\nLDCA Weight Distribution (cached):")
                    print(f"  Victim (weight > 1.2): {stats['victim_count']:,} ({stats['victim_pct']:.1f}%)")
                    print(f"  Lucky (weight < 0.9): {stats['lucky_count']:,} ({stats['lucky_pct']:.1f}%)")
                    print(f"  Agreement (0.9-1.2): {stats['agreement_count']:,} ({stats['agreement_pct']:.1f}%)")
                    print(f"  Mean weight: {stats['mean_weight']:.3f}")
                else:
                    print("Calculating LDCA statistics...")
                    import time
                    stat_start = time.time()
                    
                    weights = np.array(self.samples['weight'])
                    victim_count = int(np.sum(weights > 1.2))
                    lucky_count = int(np.sum(weights < 0.9))
                    agreement_count = int(np.sum((weights >= 0.9) & (weights <= 1.2)))
                    mean_weight = float(np.mean(weights))
                    
                    stats = {
                        'victim_count': victim_count,
                        'victim_pct': victim_count/len(self.samples)*100,
                        'lucky_count': lucky_count,
                        'lucky_pct': lucky_count/len(self.samples)*100,
                        'agreement_count': agreement_count,
                        'agreement_pct': agreement_count/len(self.samples)*100,
                        'mean_weight': mean_weight
                    }
                    
                    import json as json_module
                    with open(stats_cache, 'w') as f:
                        json_module.dump(stats, f)
                    
                    print(f"Stats calculated in {time.time() - stat_start:.2f}s")
                    print(f"\nLDCA Weight Distribution:")
                    print(f"  Victim (weight > 1.2): {victim_count:,} ({stats['victim_pct']:.1f}%)")
                    print(f"  Lucky (weight < 0.9): {lucky_count:,} ({stats['lucky_pct']:.1f}%)")
                    print(f"  Agreement (0.9-1.2): {agreement_count:,} ({stats['agreement_pct']:.1f}%)")
                    print(f"  Mean weight: {mean_weight:.3f}")
            return
        
        # Extract model name for cache filename (e.g., "Qwen/Qwen2.5-Coder-1.5B" -> "Qwen2.5-Coder-1.5B")
        model_name = tokenizer.name_or_path.split('/')[-1] if hasattr(tokenizer, 'name_or_path') else 'unknown'
        
        # Check for cached tokenized data
        cache_suffix = (
            f"_tokenized_{model_name}_maxlen{max_length}"
            f"_av{alpha_victim}_al{alpha_lucky}"
            f"_{'full' if load_full_file else 'sampled'}"
        )
        cache_ext = '.arrow' if use_arrow else '.pt'
        cache_file = str(data_file).replace('.jsonl', cache_suffix + cache_ext)
        
        if Path(cache_file).exists():
            print(f"Loading cached tokenized data from {cache_file}...")
            if use_arrow:
                # Arrow format: instant memory-mapped loading
                import time
                start = time.time()
                hf_dataset = HFDataset.load_from_disk(cache_file)
                load_time = time.time() - start
                
                # Support max_samples for smoke test
                if max_samples and len(hf_dataset) > max_samples:
                    print(f"Smoke test mode: selecting first {max_samples} samples from {len(hf_dataset):,}")
                    hf_dataset = hf_dataset.select(range(max_samples))
                
                print(f"Loaded {len(hf_dataset):,} pre-tokenized samples from Arrow cache in {load_time:.1f}s")
                
                # Enable PyTorch tensor output for zero-copy dataloading
                hf_dataset.set_format(type="torch")
                
                # Use HF Dataset directly (memory-mapped, zero-copy)
                self.samples = hf_dataset
            else:
                # Original .pt format
                self.samples = torch.load(cache_file)
                print(f"Loaded {len(self.samples):,} pre-tokenized samples from cache")
            
            # LDCA statistics with caching
            if len(self.samples) > 0:
                stats_cache = cache_file.replace('.arrow', '_ldca_stats.json').replace('.pt', '_ldca_stats.json')
                if Path(stats_cache).exists():
                    import json as json_module
                    with open(stats_cache) as f:
                        stats = json_module.load(f)
                    print(f"\nLDCA Weight Distribution (cached):")
                    print(f"  Victim (weight > 1.2): {stats['victim_count']:,} ({stats['victim_pct']:.1f}%)")
                    print(f"  Lucky (weight < 0.9): {stats['lucky_count']:,} ({stats['lucky_pct']:.1f}%)")
                    print(f"  Agreement (0.9-1.2): {stats['agreement_count']:,} ({stats['agreement_pct']:.1f}%)")
                    print(f"  Mean weight: {stats['mean_weight']:.3f}")
                else:
                    print("Calculating LDCA statistics...")
                    import time
                    stat_start = time.time()
                    
                    if use_arrow:
                        weights = np.array(self.samples['weight'])
                    else:
                        weights = np.array([s['weight'] for s in self.samples])
                    
                    victim_count = int(np.sum(weights > 1.2))
                    lucky_count = int(np.sum(weights < 0.9))
                    agreement_count = int(np.sum((weights >= 0.9) & (weights <= 1.2)))
                    mean_weight = float(np.mean(weights))
                    
                    stats = {
                        'victim_count': victim_count,
                        'victim_pct': victim_count/len(self.samples)*100,
                        'lucky_count': lucky_count,
                        'lucky_pct': lucky_count/len(self.samples)*100,
                        'agreement_count': agreement_count,
                        'agreement_pct': agreement_count/len(self.samples)*100,
                        'mean_weight': mean_weight
                    }
                    
                    import json as json_module
                    with open(stats_cache, 'w') as f:
                        json_module.dump(stats, f)
                    
                    print(f"Stats calculated in {time.time() - stat_start:.2f}s")
                    print(f"\nLDCA Weight Distribution:")
                    print(f"  Victim (weight > 1.2): {victim_count:,} ({stats['victim_pct']:.1f}%)")
                    print(f"  Lucky (weight < 0.9): {lucky_count:,} ({stats['lucky_pct']:.1f}%)")
                    print(f"  Agreement (0.9-1.2): {agreement_count:,} ({stats['agreement_pct']:.1f}%)")
                    print(f"  Mean weight: {mean_weight:.3f}")
            return
        
        print(f"Loading {data_file}...")
        print(f"No cache found - will tokenize and save to {cache_file}")
        
        if load_full_file:
            # Load all steps with trajectory context (for validation)
            print("Loading full file (all steps with trajectory context)...")
            traj_cache = defaultdict(list)
            with open(data_file) as f:
                for line in f:
                    step = json.loads(line)
                    traj_cache[step['trajectory_id']].append(step)
                    if max_samples and len(traj_cache) >= max_samples:
                        break
            
            # STEP 1: Collect all texts and metadata (fast - no tokenization yet)
            print("Step 1/3: Collecting texts and computing labels...")
            texts_to_tokenize = []
            metadata = []
            
            for traj_id, steps in tqdm(traj_cache.items(), desc="Formatting trajectories", unit="traj"):
                steps.sort(key=lambda x: x['step_position'])
                self.filter_stats['trajectories_total'] += 1
                
                traj_has_excluded = False
                traj_has_kept = False
                
                for step in steps:
                    self.filter_stats['total_steps'] += 1
                    
                    # Track filter reasons
                    if not step.get('reliable', True):
                        self.filter_stats['filtered_reliable_false'] += 1
                    if not step.get('mc_reliable', True):
                        self.filter_stats['filtered_mc_reliable_false'] += 1
                    if step.get('error_type') in ['timeout', 'sandbox_crash']:
                        self.filter_stats['filtered_timeout_sandbox'] += 1
                    if not step.get('include_in_training', True):
                        self.filter_stats['filtered_include_false'] += 1
                    
                    if not self.should_include(step):
                        traj_has_excluded = True
                        continue
                    
                    traj_has_kept = True
                    self.filter_stats['kept_steps'] += 1
                    
                    label = compute_adaptive_label(step, len(steps))
                    weight = compute_ldca_weight(step, alpha_victim=self.alpha_victim, alpha_lucky=self.alpha_lucky)
                    text = self.format_input(steps[:step['step_position']+1])
                    
                    texts_to_tokenize.append(text)
                    metadata.append({
                        'trajectory_id': traj_id,
                        'label': label,
                        'weight': weight
                    })
                
                # Track trajectory-level exclusions
                if traj_has_excluded:
                    self.filter_stats['trajectories_with_excluded'] += 1
                if not traj_has_kept:
                    self.filter_stats['trajectories_all_excluded'] += 1
            
            # STEP 2: Batch tokenize ALL texts using Rust multithreading (FAST!)
            print(f"Step 2/3: Batch tokenizing {len(texts_to_tokenize):,} samples using FastTokenizer (multithreaded)...")
            import time
            start_time = time.time()
            
            # Chunk tokenization for progress visibility
            chunk_size = 10000  # 10k samples per chunk
            all_input_ids = []
            all_attention_masks = []
            
            for i in tqdm(range(0, len(texts_to_tokenize), chunk_size), desc="Tokenizing chunks", unit="chunk"):
                chunk = texts_to_tokenize[i : i + chunk_size]
                
                # Rust backend parallelizes within each chunk
                batch_encodings = tokenizer(
                    chunk,
                    max_length=self.max_length,
                    truncation=True,
                    padding=False
                )
                
                all_input_ids.extend(batch_encodings['input_ids'])
                all_attention_masks.extend(batch_encodings['attention_mask'])
            
            tokenize_time = time.time() - start_time
            print(f"   ✅ Tokenization complete in {tokenize_time:.1f} seconds ({len(texts_to_tokenize)/tokenize_time:.0f} samples/sec)")
            
            # Create encodings dict for compatibility
            encodings = {
                'input_ids': all_input_ids,
                'attention_mask': all_attention_masks
            }
            
            # STEP 3: Assemble final dataset
            print("Step 3/3: Assembling final cached samples...")
            self.samples = []
            for i, meta in tqdm(enumerate(metadata), total=len(metadata), desc="Assembling samples", unit="sample"):
                self.samples.append({
                    'trajectory_id': meta['trajectory_id'],
                    'input_ids': encodings['input_ids'][i],
                    'attention_mask': encodings['attention_mask'][i],
                    'label': meta['label'],
                    'weight': meta['weight']
                })
            
            print(f"Loaded {len(self.samples):,} steps from {len(traj_cache):,} trajectories (full file mode)")
            
            # Print detailed filter statistics
            if self.tokenize_only:
                self._print_filter_statistics()
        else:
            # Original trajectory-based loading
            trajectories = defaultdict(list)
            with open(data_file) as f:
                for line in f:
                    step = json.loads(line)
                    trajectories[step['trajectory_id']].append(step)
                    if max_samples and len(trajectories) >= max_samples:
                        break
            
            for traj_id, steps in trajectories.items():
                steps.sort(key=lambda x: x['step_position'])
                sampled = self.sample_steps(steps)
                
                for step in sampled:
                    if not self.should_include(step):
                        continue
                    
                    label = compute_adaptive_label(step, len(steps))
                    weight = compute_ldca_weight(step, alpha_victim=self.alpha_victim, alpha_lucky=self.alpha_lucky)
                    text = self.format_input(steps[:step['step_position']+1])
                    
                    # Pre-tokenize at load time to avoid dataloader bottleneck
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
                        'weight': weight
                    })
            
            print(f"Loaded {len(self.samples):,} samples from {len(trajectories):,} trajectories")
        
        # Save tokenized data to cache
        print(f"Saving tokenized data to {cache_file}...")
        if use_arrow:
            # Save as Arrow format (memory-mapped, instant loading)
            print("Converting to Arrow format for fast loading...")
            hf_dataset = HFDataset.from_dict({
                'input_ids': [s['input_ids'] for s in self.samples],
                'attention_mask': [s['attention_mask'] for s in self.samples],
                'trajectory_id': [s['trajectory_id'] for s in self.samples],
                'label': [s['label'] for s in self.samples],
                'weight': [s['weight'] for s in self.samples],
            })
            hf_dataset.save_to_disk(cache_file)
            print(f"Arrow cache saved successfully")
            
            # Validate Arrow file
            print("Validating Arrow cache...")
            loaded = HFDataset.load_from_disk(cache_file)
            assert len(loaded) == len(self.samples), "Sample count mismatch"
            assert loaded[0]['input_ids'] == self.samples[0]['input_ids'], "Data mismatch"
            assert loaded[-1]['trajectory_id'] == self.samples[-1]['trajectory_id'], "Data mismatch"
            print("✅ Arrow cache validated")
            
            # Replace self.samples with Arrow dataset for memory efficiency
            loaded.set_format(type="torch")
            self.samples = loaded
        else:
            # Original .pt format
            torch.save(self.samples, cache_file)
            print(f"Cache saved successfully")
        
        # LDCA statistics with caching
        if len(self.samples) > 0:
            stats_cache = cache_file.replace('.arrow', '_ldca_stats.json').replace('.pt', '_ldca_stats.json')
            if Path(stats_cache).exists():
                import json as json_module
                with open(stats_cache) as f:
                    stats = json_module.load(f)
                print(f"\nLDCA Weight Distribution (cached):")
                print(f"  Victim (weight > 1.2): {stats['victim_count']:,} ({stats['victim_pct']:.1f}%)")
                print(f"  Lucky (weight < 0.9): {stats['lucky_count']:,} ({stats['lucky_pct']:.1f}%)")
                print(f"  Agreement (0.9-1.2): {stats['agreement_count']:,} ({stats['agreement_pct']:.1f}%)")
                print(f"  Mean weight: {stats['mean_weight']:.3f}")
            else:
                print("Calculating LDCA statistics...")
                import time
                stat_start = time.time()
                
                if use_arrow:
                    weights = np.array(self.samples['weight'])
                else:
                    weights = np.array([s['weight'] for s in self.samples])
                
                victim_count = int(np.sum(weights > 1.2))
                lucky_count = int(np.sum(weights < 0.9))
                agreement_count = int(np.sum((weights >= 0.9) & (weights <= 1.2)))
                mean_weight = float(np.mean(weights))
                
                stats = {
                    'victim_count': victim_count,
                    'victim_pct': victim_count/len(self.samples)*100,
                    'lucky_count': lucky_count,
                    'lucky_pct': lucky_count/len(self.samples)*100,
                    'agreement_count': agreement_count,
                    'agreement_pct': agreement_count/len(self.samples)*100,
                    'mean_weight': mean_weight
                }
                
                import json as json_module
                with open(stats_cache, 'w') as f:
                    json_module.dump(stats, f)
                
                print(f"Stats calculated in {time.time() - stat_start:.2f}s")
                print(f"\nLDCA Weight Distribution:")
                print(f"  Victim (weight > 1.2): {victim_count:,} ({stats['victim_pct']:.1f}%)")
                print(f"  Lucky (weight < 0.9): {lucky_count:,} ({stats['lucky_pct']:.1f}%)")
                print(f"  Agreement (0.9-1.2): {agreement_count:,} ({stats['agreement_pct']:.1f}%)")
                print(f"  Mean weight: {mean_weight:.3f}")
    
    def _print_filter_statistics(self):
        """Print comprehensive filter statistics"""
        total = self.filter_stats['total_steps']
        kept = self.filter_stats['kept_steps']
        excluded = total - kept
        
        # Guard against division by zero
        if total == 0:
            print(f"\n{'='*80}")
            print(f"FILTER STATISTICS")
            print(f"{'='*80}")
            print("⚠️  No steps found in dataset")
            print(f"{'='*80}\n")
            return
        
        print(f"\n{'='*80}")
        print(f"FILTER STATISTICS")
        print(f"{'='*80}")
        print(f"\n📊 OVERALL STATISTICS")
        print(f"{'─'*80}")
        print(f"Total steps:                      {total:>10,} (100.00%)")
        print(f"Kept steps (TRAINING):            {kept:>10,} ({kept/total*100:>6.2f}%)")
        print(f"Excluded steps:                   {excluded:>10,} ({excluded/total*100:>6.2f}%)")
        
        print(f"\n🔍 FILTER BREAKDOWN")
        print(f"{'─'*80}")
        print(f"reliable=False:                   {self.filter_stats['filtered_reliable_false']:>10,} ({self.filter_stats['filtered_reliable_false']/total*100:>6.2f}%)")
        print(f"mc_reliable=False:                {self.filter_stats['filtered_mc_reliable_false']:>10,} ({self.filter_stats['filtered_mc_reliable_false']/total*100:>6.2f}%)")
        print(f"error_type in [timeout, sandbox]: {self.filter_stats['filtered_timeout_sandbox']:>10,} ({self.filter_stats['filtered_timeout_sandbox']/total*100:>6.2f}%)")
        print(f"include_in_training=False:        {self.filter_stats['filtered_include_false']:>10,} ({self.filter_stats['filtered_include_false']/total*100:>6.2f}%)")
        
        print(f"\n🎯 TRAJECTORY-LEVEL IMPACT")
        print(f"{'─'*80}")
        total_trajs = self.filter_stats['trajectories_total']
        with_excluded = self.filter_stats['trajectories_with_excluded']
        all_excluded = self.filter_stats['trajectories_all_excluded']
        with_kept = total_trajs - all_excluded
        
        print(f"Total trajectories:               {total_trajs:>10,}")
        if total_trajs > 0:
            print(f"Trajectories with ≥1 excluded:    {with_excluded:>10,} ({with_excluded/total_trajs*100:>6.2f}%)")
            print(f"Trajectories with ALL excluded:   {all_excluded:>10,} ({all_excluded/total_trajs*100:>6.2f}%)")
            print(f"Trajectories with ≥1 kept:        {with_kept:>10,} ({with_kept/total_trajs*100:>6.2f}%)")
        
        print(f"\n{'='*80}")
        print(f"KEY FINDINGS")
        print(f"{'='*80}")
        print(f"✅ Data used for training:        {kept:>10,} samples ({kept/total*100:.2f}%)")
        print(f"❌ Data excluded:                 {excluded:>10,} samples ({excluded/total*100:.2f}%)")
        
        if excluded > 0:
            print(f"\n💡 Primary exclusion reasons:")
            if self.filter_stats['filtered_reliable_false'] > 0:
                print(f"   reliable=False:                {self.filter_stats['filtered_reliable_false']:>10,} ({self.filter_stats['filtered_reliable_false']/excluded*100:.2f}% of excluded)")
            if self.filter_stats['filtered_mc_reliable_false'] > 0:
                print(f"   mc_reliable=False:             {self.filter_stats['filtered_mc_reliable_false']:>10,} ({self.filter_stats['filtered_mc_reliable_false']/excluded*100:.2f}% of excluded)")
            if self.filter_stats['filtered_timeout_sandbox'] > 0:
                print(f"   timeout/sandbox_crash:         {self.filter_stats['filtered_timeout_sandbox']:>10,} ({self.filter_stats['filtered_timeout_sandbox']/excluded*100:.2f}% of excluded)")
            if self.filter_stats['filtered_include_false'] > 0:
                print(f"   include_in_training=False:     {self.filter_stats['filtered_include_false']:>10,} ({self.filter_stats['filtered_include_false']/excluded*100:.2f}% of excluded)")
        
        print(f"{'='*80}\n")
    
    def should_include(self, step):
        """Filter low-quality steps"""
        if step.get('error_type') in ['timeout', 'sandbox_crash']:
            return False
        if not step.get('include_in_training', True):
            return False
        if not step.get('reliable', True):
            return False
        return True
    
    def sample_steps(self, steps, max_steps=6):
        """Sample decision points"""
        sampled = []
        sampled_positions = set()
        
        # First 2 steps
        for step in steps[:min(2, len(steps))]:
            sampled.append(step)
            sampled_positions.add(step['step_position'])
        
        # Sharp changes
        for i in range(2, len(steps) - 1):
            if len(sampled) >= max_steps - 2:  # Leave room for error + final
                break
            
            mc_change = abs(
                safe_get_float(steps[i], 'mc_label_soft', 0.5) - 
                safe_get_float(steps[i-1], 'mc_label_soft', 0.5)
            )
            judge_change = abs(
                safe_get_float(steps[i], 'judge_overall_score', 0.5) - 
                safe_get_float(steps[i-1], 'judge_overall_score', 0.5)
            )
            
            if mc_change > 0.15 or judge_change > 0.2:
                sampled.append(steps[i])
                sampled_positions.add(steps[i]['step_position'])
        
        # First error step (important for PRM)
        for step in steps:
            judge_score = safe_get_float(step, 'judge_overall_score', 1.0)
            if judge_score < 0.4 or step.get('error_type', 'none') != 'none':
                if step['step_position'] not in sampled_positions:
                    sampled.append(step)
                    sampled_positions.add(step['step_position'])
                break
        
        # Final step
        if len(steps) > 0 and steps[-1]['step_position'] not in sampled_positions:
            sampled.append(steps[-1])
        
        return sampled[:max_steps]
    
    def format_input(self, trajectory_prefix):
        """Format trajectory as text with full context"""
        parts = [f"Task: {trajectory_prefix[0].get('task_id', 'unknown')}\n"]
        
        for step in trajectory_prefix:
            parts.append(f"\nStep {step['step_position']}:")
            parts.append(f"Action: {step['action_type']}")
            
            # Reasoning (PRIMARY signal, 99.9% coverage)
            reasoning = step.get('reasoning_normalized', step.get('reasoning', ''))
            if reasoning:
                parts.append(f"Reasoning: {reasoning[:3000]}")
            
            # Content - only when different from reasoning
            content = step.get('content', '')
            if content and str(content).strip() != str(reasoning).strip():
                parts.append(f"Content: {str(content)[:2500]}")
            
            obs = step.get('observation', {})
            if isinstance(obs, dict):
                # Compile error (direct failure signal)
                if obs.get('compile_error'):
                    parts.append(f"CompileError: {str(obs['compile_error'])[:800]}")
                
                # Test results (clearest outcome signal)
                if obs.get('test_summary'):
                    parts.append(f"Tests: {obs['test_summary']}")
                elif obs.get('tests_pass') is not None:
                    parts.append(f"TestsPass: {obs['tests_pass']}")
                
                # stderr: DROPPED (CMake boilerplate noise)
                # llm_view: DROPPED (raw file dumps, 87.8% truncated, noise)
            
            # Error type
            error_type = step.get('error_type', 'none')
            if error_type != 'none':
                parts.append(f"ErrorType: {error_type}")
        
        return "\n".join(parts)
    
    def __len__(self):
        return len(self.samples)
    
    def __getitem__(self, idx):
        sample = self.samples[idx]
        
        return {
            'input_ids': sample['input_ids'],
            'attention_mask': sample['attention_mask'],
            'trajectory_id': sample['trajectory_id'],
            'label': sample['label'],
            'weight': sample['weight']
        }


class TrajectoryBatchSampler(Sampler):
    """Sample entire trajectories, then flatten steps for ListNet grouping"""
    def __init__(self, dataset, batch_size, shuffle=True, cache_file=None):
        # Try loading from cache
        if cache_file and Path(cache_file).exists():
            print(f"Loading trajectory index from cache: {cache_file}")
            import pickle
            with open(cache_file, 'rb') as f:
                cached = pickle.load(f)
            self.traj_to_indices = cached['traj_to_indices']
            self.trajectories = cached['trajectories']
            print(f"Loaded index for {len(self.trajectories):,} trajectories in 0.1s")
        else:
            # Group step indices by trajectory
            print("Building trajectory index...")
            self.traj_to_indices = defaultdict(list)
            for idx in range(len(dataset)):
                traj_id = dataset[idx]['trajectory_id']
                self.traj_to_indices[traj_id].append(idx)
            
            self.trajectories = list(self.traj_to_indices.keys())
            print(f"Built index for {len(self.trajectories):,} trajectories")
            
            # Save to cache
            if cache_file:
                print(f"Saving trajectory index to cache: {cache_file}")
                Path(cache_file).parent.mkdir(parents=True, exist_ok=True)
                import pickle
                with open(cache_file, 'wb') as f:
                    pickle.dump({
                        'traj_to_indices': dict(self.traj_to_indices),
                        'trajectories': self.trajectories
                    }, f)
                print("Cache saved")
        
        self.batch_size = batch_size
        self.shuffle = shuffle
    
    def __iter__(self):
        trajs = self.trajectories.copy()
        if self.shuffle:
            random.shuffle(trajs)
        
        batch = []
        for traj_id in trajs:
            indices = self.traj_to_indices[traj_id]
            batch.extend(indices)
            
            # Yield when batch is large enough
            if len(batch) >= self.batch_size:
                yield batch[:self.batch_size]
                batch = batch[self.batch_size:]
        
        if batch:
            yield batch
    
    def __len__(self):
        total_steps = sum(len(indices) for indices in self.traj_to_indices.values())
        return (total_steps + self.batch_size - 1) // self.batch_size


def collate_fn(tokenizer):
    pad_token_id = tokenizer.pad_token_id
    
    def _collate(batch):
        # Sort by length descending to minimize padding waste
        batch = sorted(batch, key=lambda x: len(x['input_ids']), reverse=True)
        # Use as_tensor to avoid unnecessary copying/warnings
        input_ids = torch.nn.utils.rnn.pad_sequence(
            [torch.as_tensor(item['input_ids']) for item in batch], 
            batch_first=True, padding_value=pad_token_id
        )
        attention_mask = torch.nn.utils.rnn.pad_sequence(
            [torch.as_tensor(item['attention_mask']) for item in batch], 
            batch_first=True, padding_value=0
        )
        
        trajectory_ids = [item['trajectory_id'] for item in batch]
        labels = torch.as_tensor([item['label'] for item in batch], dtype=torch.float32)
        weights = torch.as_tensor([item['weight'] for item in batch], dtype=torch.float32)
        
        return {
            'input_ids': input_ids,
            'attention_mask': attention_mask,
            'trajectory_id': trajectory_ids,
            'labels': labels,
            'weights': weights
        }
    return _collate


# ============================================
# TRAINER WITH HYBRID LOSS
# ============================================

class HybridLossTrainer(Trainer):
    def __init__(self, *args, listnet_weight=0.7, mse_weight=0.3, progress_log_file=None, **kwargs):
        super().__init__(*args, **kwargs)
        self.listnet_weight = listnet_weight
        self.mse_weight = mse_weight
        self.log_step = 0
        self.progress_log_file = progress_log_file
        
        # LDCA stability tracking
        self.weight_stats_history = []
        self.gradient_norm_history = []
        
        # Initialize progress log file (only rank 0)
        if self.progress_log_file and self.args.process_index == 0:
            Path(self.progress_log_file).parent.mkdir(parents=True, exist_ok=True)
            with open(self.progress_log_file, 'w') as f:
                f.write("# Simple Labels + Strong LDCA PRM Training Progress\n")
                f.write(f"# Model: {kwargs.get('model', 'unknown').__class__.__name__}\n")
                f.write(f"# Loss: {listnet_weight*100:.0f}% ListNet + {mse_weight*100:.0f}% MSE\n")
                f.write("# LDCA: Strong weighting (alpha_victim=2.0, alpha_lucky=1.5)\n")
                f.write("# Metrics: ranking_accuracy, spearman, loss, weight_stats, gradient_norm\n")
                f.write("# Format: {metrics_dict}\n\n")
    
    def compute_loss(self, model, inputs, return_outputs=False, num_items_in_batch=None):
        trajectory_ids = inputs.pop('trajectory_id')
        labels = inputs.pop('labels')
        weights = inputs.pop('weights')
        
        # ============================================
        # LDCA STABILITY MONITORING
        # ============================================
        weights_np = weights.cpu().numpy()
        weight_mean = float(np.mean(weights_np))
        weight_std = float(np.std(weights_np))
        weight_min = float(np.min(weights_np))
        weight_max = float(np.max(weights_np))
        
        # Count weight distribution in batch
        victim_count = int(np.sum(weights_np > 1.2))
        lucky_count = int(np.sum(weights_np < 0.9))
        agreement_count = int(np.sum((weights_np >= 0.9) & (weights_np <= 1.2)))
        
        # Track for stability analysis
        self.weight_stats_history.append({
            'mean': weight_mean,
            'std': weight_std,
            'min': weight_min,
            'max': weight_max,
            'victim_pct': victim_count / len(weights_np) * 100,
            'lucky_pct': lucky_count / len(weights_np) * 100,
        })
        
        # Keep only last 1000 steps
        if len(self.weight_stats_history) > 1000:
            self.weight_stats_history.pop(0)
        
        # Forward
        pred_scores = model(**inputs)
        
        # Convert trajectory_ids to tensor for grouping
        unique_trajs = list(set(trajectory_ids))
        traj_to_idx = {t: i for i, t in enumerate(unique_trajs)}
        group_ids = torch.tensor([traj_to_idx[t] for t in trajectory_ids], device=pred_scores.device)
        
        # ============================================
        # 1. ListNet Loss (50%)
        # ============================================
        listnet_loss = 0.0
        n_groups = 0
        
        for group_id in range(len(unique_trajs)):
            mask = (group_ids == group_id)
            
            if mask.sum() <= 1:
                continue
            
            group_pred = pred_scores[mask]
            group_target = labels[mask].to(pred_scores.device)
            group_weight = weights[mask].mean().to(pred_scores.device)
            
            # ListNet: KL divergence (normalized by group size)
            p_true = F.softmax(group_target, dim=0)
            p_pred = F.log_softmax(group_pred, dim=0)
            
            group_loss = -torch.sum(p_true * p_pred) / mask.sum().float()  # Normalize by group size
            listnet_loss += group_weight * group_loss
            n_groups += 1
        
        if n_groups > 0:
            listnet_loss = listnet_loss / n_groups
        else:
            listnet_loss = torch.tensor(0.0, device=pred_scores.device)
        
        # ============================================
        # 2. MSE Loss (50%)
        # ============================================
        mse_loss = (weights.to(pred_scores.device) * (pred_scores - labels.to(pred_scores.device)) ** 2).mean()
        
        # ============================================
        # 3. Combined Loss
        # ============================================
        total_loss = self.listnet_weight * listnet_loss + self.mse_weight * mse_loss
        
        # ============================================
        # GRADIENT NORM MONITORING (after backward)
        # ============================================
        # Note: Gradient norm will be computed in training_step callback
        
        # Log loss components periodically
        self.log_step += 1
        if self.log_step % 50 == 0:
            log_dict = {
                "train/listnet_loss": listnet_loss.item(),
                "train/mse_loss": mse_loss.item(),
                "train/n_groups": n_groups,
                "train/total_loss": total_loss.item(),
                # LDCA weight statistics
                "train/weight_mean": weight_mean,
                "train/weight_std": weight_std,
                "train/weight_min": weight_min,
                "train/weight_max": weight_max,
                "train/victim_pct": victim_count / len(weights_np) * 100,
                "train/lucky_pct": lucky_count / len(weights_np) * 100,
                "train/agreement_pct": agreement_count / len(weights_np) * 100,
            }
            
            # Add gradient norm if available
            if len(self.gradient_norm_history) > 0:
                recent_grad_norms = [g['total_norm'] for g in self.gradient_norm_history[-50:]]
                log_dict["train/grad_norm_mean"] = float(np.mean(recent_grad_norms))
                log_dict["train/grad_norm_max"] = float(np.max(recent_grad_norms))
            
            self.log(log_dict)
            
            # Write to progress log (only rank 0)
            if self.progress_log_file and self.args.process_index == 0:
                with open(self.progress_log_file, 'a') as f:
                    f.write(f"{log_dict}\n")
            
            # ============================================
            # STABILITY WARNINGS
            # ============================================
            if self.args.process_index == 0:  # Only rank 0 prints warnings
                # Warning: Extreme weight imbalance
                if weight_std > 0.5:
                    print(f"⚠️  [Step {self.log_step}] High weight variance: std={weight_std:.3f} (mean={weight_mean:.3f})")
                    print(f"   Victim: {victim_count}/{len(weights_np)} ({victim_count/len(weights_np)*100:.1f}%)")
                    print(f"   Lucky: {lucky_count}/{len(weights_np)} ({lucky_count/len(weights_np)*100:.1f}%)")
                
                # Warning: Victim-heavy batch
                if victim_count / len(weights_np) > 0.8:
                    print(f"⚠️  [Step {self.log_step}] Victim-heavy batch: {victim_count/len(weights_np)*100:.1f}% victim steps")
                    print(f"   This may cause gradient spikes. Consider gradient clipping.")
                
                # Warning: High gradient norm
                if len(self.gradient_norm_history) > 0:
                    recent_norm = self.gradient_norm_history[-1]['total_norm']
                    if recent_norm > 10.0:
                        print(f"⚠️  [Step {self.log_step}] High gradient norm: {recent_norm:.2f}")
                        print(f"   Weight stats: mean={weight_mean:.3f}, std={weight_std:.3f}, max={weight_max:.3f}")
        
        return (total_loss, pred_scores) if return_outputs else total_loss
    
    def evaluate(self, eval_dataset=None, ignore_keys=None, metric_key_prefix="eval"):
        """Custom evaluation with ranking metrics (single-pass, DDP-safe, distributed eval)"""
        model = self.model
        model.eval()
        
        eval_dataset = eval_dataset or self.eval_dataset
        
        # Use distributed eval dataloader (sharded across GPUs)
        dataloader = self.get_eval_dataloader(eval_dataset)
        
        all_preds = []
        all_labels = []
        all_traj_ids = []
        total_loss = 0.0
        n_batches = 0
        
        # Distributed evaluation: each GPU processes its shard
        with torch.no_grad():
            for batch_idx, batch in enumerate(dataloader):
                batch = {k: v.to(self.args.device) if isinstance(v, torch.Tensor) else v 
                        for k, v in batch.items()}
                
                traj_ids = batch.pop('trajectory_id')
                labels = batch.pop('labels')
                weights = batch.pop('weights')
                
                preds = model(**batch)
                
                # Compute eval loss (MSE)
                mse = (weights.to(preds.device) * (preds - labels.to(preds.device)) ** 2).mean()
                total_loss += mse.item()
                n_batches += 1
                
                all_preds.extend(preds.cpu().float().numpy())
                all_labels.extend(labels.cpu().float().numpy())
                all_traj_ids.extend(traj_ids)
                
                # Progress indicator (rank 0 only, every 50 batches)
                if self.args.process_index == 0 and batch_idx % 50 == 0:
                    print(f"[Eval] Rank 0: {batch_idx}/{len(dataloader)} batches processed", flush=True)
        
        # DDP: Gather across all ranks for correct metrics
        if self.args.world_size > 1:
            import torch.distributed as dist
            
            gathered_preds = [None] * self.args.world_size
            gathered_labels = [None] * self.args.world_size
            gathered_ids = [None] * self.args.world_size
            gathered_loss = [None] * self.args.world_size
            
            # Use all_gather_object for CPU data (no NCCL timeout)
            dist.all_gather_object(gathered_preds, all_preds)
            dist.all_gather_object(gathered_labels, all_labels)
            dist.all_gather_object(gathered_ids, all_traj_ids)
            dist.all_gather_object(gathered_loss, (total_loss, n_batches))
            
            all_preds = [x for sub in gathered_preds for x in sub]
            all_labels = [x for sub in gathered_labels for x in sub]
            all_traj_ids = [x for sub in gathered_ids for x in sub]
            total_loss = sum(t for t, _ in gathered_loss)
            n_batches = sum(n for _, n in gathered_loss)
        
        # Compute ranking accuracy (only on rank 0 to avoid redundant computation)
        if self.args.process_index == 0:
            from collections import defaultdict
            traj_groups = defaultdict(list)
            for pred, label, traj_id in zip(all_preds, all_labels, all_traj_ids):
                traj_groups[traj_id].append((pred, label))
            
            correct_rankings = 0
            total_pairs = 0
            
            for traj_id, items in traj_groups.items():
                if len(items) < 2:
                    continue
                
                # Check pairwise ranking accuracy
                for i in range(len(items)):
                    for j in range(i + 1, len(items)):
                        pred_i, label_i = items[i]
                        pred_j, label_j = items[j]
                        
                        # If label_i > label_j, then pred_i should > pred_j
                        if label_i != label_j:
                            if (label_i > label_j) == (pred_i > pred_j):
                                correct_rankings += 1
                            total_pairs += 1
            
            ranking_accuracy = correct_rankings / total_pairs if total_pairs > 0 else 0.0
            avg_loss = total_loss / n_batches if n_batches > 0 else 0.0
            
            # Compute Spearman correlation (if scipy available)
            spearman_corr = 0.0
            if HAS_SCIPY and len(all_preds) > 1:
                spearman_corr, _ = spearmanr(all_preds, all_labels)
                if np.isnan(spearman_corr):
                    spearman_corr = 0.0
                else:
                    spearman_corr = float(spearman_corr)  # Convert numpy float to Python float
            
            metrics = {
                f"{metric_key_prefix}_loss": avg_loss,
                f"{metric_key_prefix}_ranking_accuracy": ranking_accuracy,
                f"{metric_key_prefix}_spearman": spearman_corr,
            }
        else:
            # Non-rank-0 processes: create dummy metrics
            metrics = {
                f"{metric_key_prefix}_loss": 0.0,
                f"{metric_key_prefix}_ranking_accuracy": 0.0,
                f"{metric_key_prefix}_spearman": 0.0,
            }
        
        # DDP: Broadcast metrics from rank 0 to all ranks
        if self.args.world_size > 1:
            import torch.distributed as dist
            metrics_list = [metrics]
            dist.broadcast_object_list(metrics_list, src=0)
            metrics = metrics_list[0]
        
        self.log(metrics)
        
        # Write to progress log (only rank 0)
        if self.progress_log_file and self.args.process_index == 0:
            with open(self.progress_log_file, 'a') as f:
                f.write(f"{metrics}\n")
        
        return metrics


# ============================================
# MAIN
# ============================================

def main():
    import torch.multiprocessing as mp
    mp.set_start_method('fork', force=True)  # Prevents SIGSEGV with Arrow + multiprocessing
    parser = argparse.ArgumentParser()
    parser.add_argument('--no_lora', action='store_true', help='Disable LoRA (use full fine-tuning)')
    parser.add_argument('--freeze_encoder', action='store_true', help='Freeze encoder (train head only)')
    parser.add_argument('--smoke_test', action='store_true', help='Quick test with small data')
    parser.add_argument('--alpha_victim', type=float, default=2.0, 
                        help='LDCA amplification for victim steps (default: 2.0, was 1.3)')
    parser.add_argument('--alpha_lucky', type=float, default=1.5,
                        help='LDCA dampening for lucky steps (default: 1.5, was 0.7)')
    parser.add_argument('--disable_ldca', action='store_true',
                        help='Disable LDCA (use uniform weighting)')
    parser.add_argument('--base_model', type=str, default='Qwen/Qwen2.5-Coder-1.5B',
                        help='Base model (Qwen2.5-Coder-1.5B: strong code model, efficient size)')
    parser.add_argument('--batch_size', type=int, default=8,
                        help='Per-device batch size (Optimized for H100 6K context)')
    parser.add_argument('--grad_accum', type=int, default=4,
                        help='Gradient accumulation steps (Effective batch = 64)')
    parser.add_argument('--resume_from_checkpoint', type=str, default=None,
                        help='Path to checkpoint to resume from')
    parser.add_argument('--tokenize_only', action='store_true',
                        help='Only tokenize data and exit')
    parser.add_argument('--epochs', type=int, default=1,
                        help='Number of training epochs')
    parser.add_argument('--max_length', type=int, default=6144,
                        help='Maximum sequence length (default: 6144, reasoning=3000 with stderr+llm_view dropped)')
    parser.add_argument('--use_arrow', action='store_true',
                        help='Use Arrow format for instant cache loading')
    parser.add_argument('--use_qlora', action='store_true',
                        help='Use QLoRA (4-bit quantization) for memory efficiency')
    parser.add_argument('--eval_only', action='store_true',
                        help='Only run evaluation on validation set (no training)')
    parser.add_argument('--eval_checkpoint', type=str, default=None,
                        help='Checkpoint to evaluate (for --eval_only mode)')
    parser.add_argument('--test_set', action='store_true',
                        help='Evaluate on test set instead of validation set (use with --eval_only)')
    args = parser.parse_args()
    
    # --- HARDWARE DETECTION ---
    is_h100 = torch.cuda.is_available() and "H100" in torch.cuda.get_device_name(0)
    is_mps = torch.backends.mps.is_available()
    device_name = (
        "H100" if is_h100
        else torch.cuda.get_device_name(0) if torch.cuda.is_available()
        else ("MPS (Apple Silicon)" if is_mps else "CPU")
    )

    # --- CONFIGURATION ---
    config = {
        'base_model': args.base_model,
        'use_lora': not args.no_lora,
        'freeze_encoder': args.freeze_encoder,
        'train_file': 'path/t3_cpp/data_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/train.jsonl',
        'val_file': 'path/t3_cpp/data_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/val.jsonl',
        'test_file': 'path/t3_cpp/data_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl',
        'output_dir': 'path/t3_cpp/data_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE/outputs_simple_labels_strong_ldca',
        'batch_size': args.batch_size,
        'gradient_accumulation': args.grad_accum,
        'learning_rate': 2e-5 if not args.no_lora else 1e-5,
        'epochs': args.epochs,
        'max_length': args.max_length, 
        'listnet_weight': 0.5,  # CHANGED: 0.7 → 0.5 (LDCA primarily affects MSE)
        'mse_weight': 0.5,      # CHANGED: 0.3 → 0.5 (increased for stronger LDCA effect)
    }
    
    if args.smoke_test:
        config['epochs'] = 1
        config['batch_size'] = 2
        config['gradient_accumulation'] = 2
        config['output_dir'] += '_smoke'
        config['save_steps'] = 2
        config['eval_steps'] = 2
    
    # Store LDCA params in config
    config['alpha_victim'] = args.alpha_victim if not args.disable_ldca else 0.0
    config['alpha_lucky'] = args.alpha_lucky if not args.disable_ldca else 0.0
    
    print("="*70)
    print("🚀 SIMPLE LABELS + STRONG LDCA PRM TRAINING")
    print("="*70)
    print(f"Model: {config['base_model']}")
    print(f"Device: {device_name}")
    print(f"Batch size: {config['batch_size']} | Grad Accum: {config['gradient_accumulation']}")
    print(f"Effective Batch: {config['batch_size'] * config['gradient_accumulation']}")
    print(f"Loss: {config['listnet_weight']*100:.0f}% ListNet + {config['mse_weight']*100:.0f}% MSE")
    print(f"LDCA: alpha_victim={config['alpha_victim']:.1f}, alpha_lucky={config['alpha_lucky']:.1f}")
    print(f"Labels: Simple 50-50 blend (no adaptive heuristics)")
    if torch.cuda.is_available():
        print(f"Precision: BF16 + TF32 (Hopper Optimized)")
    else:
        print(f"Precision: FP32 (MPS/CPU)")
    print("="*70)
    
    # Load tokenizer and model
    tokenizer = AutoTokenizer.from_pretrained(config['base_model'])
    tokenizer.pad_token = tokenizer.eos_token
    
    model = AdaptivePRM(
        base_model=config['base_model'],
        use_lora=config['use_lora'],
        freeze_encoder=config['freeze_encoder'],
        use_qlora=args.use_qlora
    )
    
    # --- DATASET LOADING ---
    dataset_kwargs = {
        "tokenizer": tokenizer,
        "max_length": config['max_length'],
        "alpha_victim": config['alpha_victim'],
        "alpha_lucky": config['alpha_lucky'],
        "load_full_file": True,
        "use_arrow": args.use_arrow,
    }

    if args.tokenize_only:
        print("\n" + "="*70)
        print("TOKENIZING ALL SPLITS (--tokenize_only mode)")
        print("="*70)
        splits = [
            ('TRAIN', config['train_file']),
            ('VAL', config['val_file']),
            ('TEST', config['test_file']),
        ]
        for split_name, split_file in splits:
            print(f"\nTOKENIZING {split_name} SPLIT: {split_file}")
            PRMDataset(split_file, tokenize_only=True, **dataset_kwargs)
        return
    
    # EVAL ONLY MODE: Load checkpoint and run evaluation (DDP-aware)
    if args.eval_only:
        # DDP: Initialize process group for distributed evaluation
        import torch.distributed as dist
        import os as os_module
        is_distributed = dist.is_available() and "RANK" in os_module.environ
        if is_distributed:
            dist.init_process_group(backend="nccl", timeout=timedelta(hours=2))
            torch.cuda.set_device(int(os_module.environ["LOCAL_RANK"]))
            rank = dist.get_rank()
            world_size = dist.get_world_size()
        else:
            rank = 0
            world_size = 1
        
        # Check if evaluating base model (untrained)
        is_base_model = args.eval_checkpoint and args.eval_checkpoint.lower() == 'base'
        
        if rank == 0:
            print("\n" + "="*70)
            print("EVALUATION ONLY MODE (DDP-AWARE)")
            print("="*70)
            if is_base_model:
                print("🔬 Evaluating UNTRAINED BASE MODEL")
            if args.test_set:
                print("📊 Evaluating on TEST SET")
            else:
                print("📊 Evaluating on VALIDATION SET")
        
        if not is_base_model and args.eval_checkpoint is None:
            # Find latest checkpoint
            output_path = Path(config['output_dir'])
            if output_path.exists():
                checkpoints = sorted(output_path.glob('checkpoint-*'), key=lambda x: int(x.name.split('-')[1]))
                valid_checkpoints = [cp for cp in checkpoints if (cp / 'trainer_state.json').exists()]
                if valid_checkpoints:
                    args.eval_checkpoint = str(valid_checkpoints[-1])
                    if rank == 0:
                        print(f"Auto-detected latest checkpoint: {args.eval_checkpoint}")
                else:
                    if rank == 0:
                        print("❌ No valid checkpoints found")
                    return
            else:
                if rank == 0:
                    print(f"❌ Output directory does not exist: {config['output_dir']}")
                return
        
        if rank == 0:
            if is_base_model:
                print(f"Using untrained base model: {config['base_model']}")
            else:
                print(f"Loading checkpoint: {args.eval_checkpoint}")
        
        # Load validation or test dataset
        eval_file = config['test_file'] if args.test_set else config['val_file']
        
        # DDP: Rank 0 tokenizes first, others wait for cache
        if rank == 0:
            print(f"\nLoading {'test' if args.test_set else 'validation'} dataset...")
            print(f"Dataset file: {eval_file}")
            eval_dataset = PRMDataset(eval_file, **dataset_kwargs)
        
        # Barrier: Wait for rank 0 to finish tokenization
        if is_distributed:
            if rank == 0:
                print("Rank 0: Dataset prep done, signalling barrier...")
            else:
                print(f"Rank {rank}: Waiting at barrier for rank 0 to finish dataset prep...")
            import torch.distributed as dist
            # Use gloo for CPU barrier (no NCCL timeout)
            gloo_group = dist.new_group(backend="gloo", timeout=timedelta(hours=2))
            dist.barrier(group=gloo_group)
            print(f"Rank {rank}: Cache ready, loading dataset...")
        
        # All non-rank-0 processes load from cache
        if rank != 0:
            print(f"\nRank {rank}: Loading {'test' if args.test_set else 'validation'} dataset from cache...")
            eval_dataset = PRMDataset(eval_file, **dataset_kwargs)
        
        # Load model and checkpoint
        if rank == 0:
            print("\nLoading model...")
        device = torch.device(f'cuda:{rank}' if torch.cuda.is_available() else 'cpu')
        
        if not is_base_model:
            # Load checkpoint state
            checkpoint_path = Path(args.eval_checkpoint)
            if rank == 0:
                print(f"Loading checkpoint from {checkpoint_path}/model.safetensors...")
            
            from safetensors.torch import load_file
            state_dict = load_file(str(checkpoint_path / 'model.safetensors'), device=str(device))
            
            # Load into model (handles PEFT wrapping correctly)
            model.load_state_dict(state_dict, strict=False)
            model.to(device)
            
            # Convert to bfloat16 to match training dtype
            if torch.cuda.is_available():
                model = model.to(torch.bfloat16)
            
            # Disable gradient checkpointing for eval (efficiency)
            if hasattr(model, 'gradient_checkpointing_disable'):
                model.gradient_checkpointing_disable()
            
            model.eval()
            if rank == 0:
                print(f"✅ Checkpoint loaded from {args.eval_checkpoint}")
        else:
            # Use untrained base model
            model.to(device)
            
            # Convert to bfloat16 to match training dtype
            if torch.cuda.is_available():
                model = model.to(torch.bfloat16)
            
            # Disable gradient checkpointing for eval (efficiency)
            if hasattr(model, 'gradient_checkpointing_disable'):
                model.gradient_checkpointing_disable()
            
            model.eval()
            if rank == 0:
                print(f"✅ Using untrained base model: {config['base_model']}")
        
        # Create DDP-aware dataloader (sharded across GPUs)
        from torch.utils.data import DataLoader, DistributedSampler
        if is_distributed:
            sampler = DistributedSampler(eval_dataset, num_replicas=world_size, rank=rank, shuffle=False)
            dataloader = DataLoader(
                eval_dataset,
                batch_size=config['batch_size'],
                sampler=sampler,
                collate_fn=collate_fn(tokenizer),
                num_workers=4,
                pin_memory=True,
                prefetch_factor=2,
            )
        else:
            dataloader = DataLoader(
                eval_dataset,
                batch_size=config['batch_size'],
                shuffle=False,
                collate_fn=collate_fn(tokenizer),
                num_workers=4,
                pin_memory=True,
                prefetch_factor=2,
            )
        
        # Run evaluation (DDP-aware: each GPU processes its shard)
        if rank == 0:
            print(f"\nRunning evaluation on {world_size} GPU(s)...")
        all_preds = []
        all_labels = []
        all_traj_ids = []
        all_weights = []
        total_loss = 0.0
        n_batches = 0
        
        with torch.no_grad():
            for batch_idx, batch in enumerate(tqdm(dataloader, desc=f"Evaluating (Rank {rank})", disable=(rank != 0))):
                batch = {k: v.to(device) if isinstance(v, torch.Tensor) else v 
                        for k, v in batch.items()}
                
                traj_ids = batch.pop('trajectory_id')
                labels = batch.pop('labels')
                weights = batch.pop('weights')
                
                preds = model(**batch)
                
                # Compute loss
                mse = (weights.to(preds.device) * (preds - labels.to(preds.device)) ** 2).mean()
                total_loss += mse.item()
                n_batches += 1
                
                all_preds.extend(preds.cpu().float().numpy())
                all_labels.extend(labels.cpu().float().numpy())
                all_traj_ids.extend(traj_ids)
                all_weights.extend(weights.cpu().float().numpy())
        
        # DDP: Gather data from all ranks
        if is_distributed:
            gathered_preds = [None] * world_size
            gathered_labels = [None] * world_size
            gathered_ids = [None] * world_size
            gathered_weights = [None] * world_size
            gathered_loss = [None] * world_size
            
            dist.all_gather_object(gathered_preds, all_preds)
            dist.all_gather_object(gathered_labels, all_labels)
            dist.all_gather_object(gathered_ids, all_traj_ids)
            dist.all_gather_object(gathered_weights, all_weights)
            dist.all_gather_object(gathered_loss, (total_loss, n_batches))
            
            all_preds = [x for sub in gathered_preds for x in sub]
            all_labels = [x for sub in gathered_labels for x in sub]
            all_traj_ids = [x for sub in gathered_ids for x in sub]
            all_weights = [x for sub in gathered_weights for x in sub]
            total_loss = sum(t for t, _ in gathered_loss)
            n_batches = sum(n for _, n in gathered_loss)
        # Compute overall metrics (only on rank 0)
        if rank == 0:
            print("\nComputing metrics...")
        from collections import defaultdict
        traj_groups = defaultdict(list)
        for pred, label, traj_id in zip(all_preds, all_labels, all_traj_ids):
            traj_groups[traj_id].append((pred, label))
        
        correct_rankings = 0
        total_pairs = 0
        
        for traj_id, items in traj_groups.items():
            if len(items) < 2:
                continue
            
            for i in range(len(items)):
                for j in range(i + 1, len(items)):
                    pred_i, label_i = items[i]
                    pred_j, label_j = items[j]
                    
                    if label_i != label_j:
                        if (label_i > label_j) == (pred_i > pred_j):
                            correct_rankings += 1
                        total_pairs += 1
        
        ranking_accuracy = correct_rankings / total_pairs if total_pairs > 0 else 0.0
        avg_loss = total_loss / n_batches if n_batches > 0 else 0.0
        
        spearman_corr = 0.0
        if HAS_SCIPY and len(all_preds) > 1:
            spearman_corr, _ = spearmanr(all_preds, all_labels)
            if np.isnan(spearman_corr):
                spearman_corr = 0.0
            else:
                spearman_corr = float(spearman_corr)
        
        # Stratify by LDCA weight
        print("\nStratifying by LDCA weight...")
        victim_preds, victim_labels, victim_traj_ids, victim_weights = [], [], [], []
        lucky_preds, lucky_labels, lucky_traj_ids, lucky_weights = [], [], [], []
        agreement_preds, agreement_labels, agreement_traj_ids, agreement_weights = [], [], [], []
        
        # Stratify by LDCA weight
        print("\nStratifying by LDCA weight...")
        victim_preds, victim_labels, victim_traj_ids, victim_weights = [], [], [], []
        lucky_preds, lucky_labels, lucky_traj_ids, lucky_weights = [], [], [], []
        agreement_preds, agreement_labels, agreement_traj_ids, agreement_weights = [], [], [], []
        
        for pred, label, traj_id, weight in zip(all_preds, all_labels, all_traj_ids, all_weights):
            if weight > 1.2:
                victim_preds.append(pred)
                victim_labels.append(label)
                victim_traj_ids.append(traj_id)
                victim_weights.append(weight)
            elif weight < 0.9:
                lucky_preds.append(pred)
                lucky_labels.append(label)
                lucky_traj_ids.append(traj_id)
                lucky_weights.append(weight)
            else:
                agreement_preds.append(pred)
                agreement_labels.append(label)
                agreement_traj_ids.append(traj_id)
                agreement_weights.append(weight)
        
        # Compute stratified metrics
        stratified_results = {}
        
        for stratum_name, (s_preds, s_labels, s_traj_ids, s_weights) in [
            ('victim', (victim_preds, victim_labels, victim_traj_ids, victim_weights)),
            ('lucky', (lucky_preds, lucky_labels, lucky_traj_ids, lucky_weights)),
            ('agreement', (agreement_preds, agreement_labels, agreement_traj_ids, agreement_weights)),
        ]:
            if len(s_preds) == 0:
                continue
            
            # Ranking accuracy
            s_traj_groups = defaultdict(list)
            for pred, label, traj_id in zip(s_preds, s_labels, s_traj_ids):
                s_traj_groups[traj_id].append((pred, label))
            
            s_correct = 0
            s_total = 0
            for traj_id, items in s_traj_groups.items():
                if len(items) < 2:
                    continue
                for i in range(len(items)):
                    for j in range(i + 1, len(items)):
                        pred_i, label_i = items[i]
                        pred_j, label_j = items[j]
                        if label_i != label_j:
                            if (label_i > label_j) == (pred_i > pred_j):
                                s_correct += 1
                            s_total += 1
            
            s_ranking_acc = s_correct / s_total if s_total > 0 else 0.0
            s_mse = np.mean(np.array(s_weights) * (np.array(s_preds) - np.array(s_labels)) ** 2)
            
            s_spearman = 0.0
            if HAS_SCIPY and len(s_preds) > 1:
                s_spearman, _ = spearmanr(s_preds, s_labels)
                if np.isnan(s_spearman):
                    s_spearman = 0.0
                else:
                    s_spearman = float(s_spearman)
            
            stratified_results[stratum_name] = {
                'n_samples': len(s_preds),
                'ranking_accuracy': s_ranking_acc,
                'mse': float(s_mse),
                'spearman': s_spearman,
                'mean_pred': float(np.mean(s_preds)),
                'mean_label': float(np.mean(s_labels)),
                'mean_weight': float(np.mean(s_weights)),
            }
        
        # Print results
        print("\n" + "="*70)
        print("OVERALL RESULTS")
        print("="*70)
        print(f"Samples:          {len(all_preds):,}")
        print(f"Ranking Accuracy: {ranking_accuracy:.4f}")
        print(f"MSE:              {avg_loss:.6f}")
        print(f"Spearman:         {spearman_corr:.4f}")
        
        print("\n" + "="*70)
        print("STRATIFIED RESULTS")
        print("="*70)
        for stratum_name in ['victim', 'lucky', 'agreement']:
            if stratum_name not in stratified_results:
                continue
            s = stratified_results[stratum_name]
            print(f"\n{stratum_name.upper()} ({s['n_samples']:,} samples, {s['n_samples']/len(all_preds)*100:.1f}%):")
            print(f"  Ranking Accuracy: {s['ranking_accuracy']:.4f}")
            print(f"  MSE:              {s['mse']:.6f}")
            print(f"  Spearman:         {s['spearman']:.4f}")
            print(f"  Mean Pred:        {s['mean_pred']:.4f}")
            print(f"  Mean Label:       {s['mean_label']:.4f}")
            print(f"  Mean Weight:      {s['mean_weight']:.4f}")
        
        # Save results
        results = {
            'checkpoint': 'base_model_untrained' if is_base_model else args.eval_checkpoint,
            'base_model': config['base_model'],
            'overall': {
                'n_samples': len(all_preds),
                'ranking_accuracy': ranking_accuracy,
                'mse': avg_loss,
                'spearman': spearman_corr,
            },
            'stratified': stratified_results,
        }
        
        # Save results to appropriate directory
        if is_base_model:
            # Save to dedicated base model results directory
            base_results_dir = Path('path/t3_cpp/data_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE/stratified_results_basemodel')
            base_results_dir.mkdir(parents=True, exist_ok=True)
            split_name = 'test' if args.test_set else 'val'
            output_file = base_results_dir / f'stratified_eval_results_{split_name}_base_model.json'
            log_file = base_results_dir / f'eval_{split_name}_base_model.log'
        else:
            # Save to checkpoint's parent directory (not config output_dir)
            checkpoint_parent = checkpoint_path.parent
            split_name = 'test' if args.test_set else 'val'
            output_file = checkpoint_parent / f'stratified_eval_results_{split_name}_{checkpoint_path.name}.json'
            log_file = checkpoint_parent / f'eval_{split_name}_{checkpoint_path.name}.log'
        
        with open(output_file, 'w') as f:
            json.dump(results, f, indent=2)
        
        print(f"\n✅ Results saved to: {output_file}")
        print("="*70)
        return
    
    # DDP: Initialize process group BEFORE dataset loading so barrier works
    import torch.distributed as dist
    import os as os_module
    is_distributed = dist.is_available() and "RANK" in os.environ
    if is_distributed:
        # Increase timeout to 2 HOURS for gradient checkpointing on 6144-length sequences
        os_module.environ.setdefault('GLOO_SOCKET_IFNAME', 'lo')  # Use loopback for stability
        # Use gloo for the tokenization barrier (CPU-only, no NCCL timeout issues)
        dist.init_process_group(backend="nccl", timeout=timedelta(hours=2))
        torch.cuda.set_device(int(os.environ["LOCAL_RANK"]))
        # Separate gloo group for CPU barriers (avoids NCCL 600s timeout)
        gloo_group = dist.new_group(backend="gloo", timeout=timedelta(hours=2))
    rank = dist.get_rank() if is_distributed else 0
    
    if rank == 0:
        print("\nLoading training dataset (FULL dataset)...")
        train_dataset = PRMDataset(config['train_file'], **dataset_kwargs)
        
        print("\nLoading validation dataset...")
        val_dataset = PRMDataset(config['val_file'], **dataset_kwargs)
    
    # Barrier: Wait for rank 0 to finish tokenization (use gloo - no timeout)
    if is_distributed:
        if rank == 0:
            print("Rank 0: Dataset prep done, signalling barrier...")
        else:
            print(f"Rank {rank}: Waiting at barrier for rank 0 to finish dataset prep...")
        dist.barrier(group=gloo_group)
        print(f"Rank {rank}: Cache ready, loading datasets...")
    
    # All ranks load from cache
    if rank != 0:
        print("\nLoading training dataset (from cache)...")
        train_dataset = PRMDataset(config['train_file'], **dataset_kwargs)
        
        print("\nLoading validation dataset (from cache)...")
        val_dataset = PRMDataset(config['val_file'], **dataset_kwargs)
    
    # --- TRAINING ARGUMENTS ---
    training_args = TrainingArguments(
        output_dir=config['output_dir'],
        num_train_epochs=config['epochs'],
        # Note: max_steps will be ~len(train_sampler)//world_size per epoch
        # Trainer computes this correctly once get_train_dataloader is overridden
        per_device_train_batch_size=config['batch_size'],
        per_device_eval_batch_size=config['batch_size'],
        gradient_accumulation_steps=config['gradient_accumulation'],
        
        # Optimizer & Schedule
        optim="adamw_torch" if is_mps else "adamw_torch_fused",  # Fused not available on MPS
        learning_rate=config['learning_rate'],
        weight_decay=0.01,
        lr_scheduler_type="cosine",
        warmup_steps=100 if args.smoke_test else 500,
        
        # Precision & Memory
        bf16=torch.cuda.is_available(),  # BF16 only on CUDA
        fp16=False,
        tf32=torch.cuda.is_available(),  # TF32 only on CUDA
        gradient_checkpointing=True,
        gradient_checkpointing_kwargs={"use_reentrant": False},
        
        # Gradient clipping for LDCA stability
        max_grad_norm=1.0,  # Clip gradients to prevent spikes from victim-heavy batches
        
        # Logging & Evaluation
        logging_steps=10 if args.smoke_test else 50,
        logging_first_step=True,
        eval_strategy="steps",
        eval_steps=config.get('eval_steps', 1000),
        save_strategy="steps",
        save_steps=config.get('save_steps', 1000),
        save_total_limit=None,
        load_best_model_at_end=True,
        metric_for_best_model="eval_ranking_accuracy",
        greater_is_better=True,
        
        # Speed & Stability
        dataloader_num_workers=4,
        dataloader_pin_memory=True,
        dataloader_prefetch_factor=2,
        ddp_find_unused_parameters=False,
        ddp_backend="nccl",
        remove_unused_columns=False,
        report_to="none",
        
        # Evaluation optimization
        eval_accumulation_steps=2,  # Accumulate eval batches to reduce sync overhead
    )
    
    # Create trajectory-aware batch sampler (rank 0 builds, others load from cache)
    print("\nCreating trajectory-aware batch sampler...")
    sampler_cache = f"{config['output_dir']}/trajectory_sampler_cache.pkl"
    if is_distributed and rank != 0:
        # Wait for rank 0 to build the cache
        dist.barrier(group=gloo_group)
        train_sampler = TrajectoryBatchSampler(
            train_dataset,
            batch_size=config['batch_size'],
            shuffle=True,
            cache_file=sampler_cache
        )
    else:
        train_sampler = TrajectoryBatchSampler(
            train_dataset,
            batch_size=config['batch_size'],
            shuffle=True,
            cache_file=sampler_cache
        )
        if is_distributed:
            dist.barrier(group=gloo_group)  # Signal other ranks that cache is ready
    print(f"Sampler created: {len(train_sampler)} batches from {len(train_sampler.trajectories):,} trajectories")
    
    if torch.cuda.device_count() > 1:
        print(f"Enabling gradient checkpointing for {torch.cuda.device_count()} GPUs...")
        model.gradient_checkpointing_enable(gradient_checkpointing_kwargs={"use_reentrant": False})
        print("✅ Gradient checkpointing enabled")
    
    # Trainer
    progress_log_file = f"{config['output_dir']}/progress.log"
    trainer = HybridLossTrainer(
        model=model,
        args=training_args,
        train_dataset=train_dataset,
        eval_dataset=val_dataset,
        data_collator=collate_fn(tokenizer),
        listnet_weight=config['listnet_weight'],
        mse_weight=config['mse_weight'],
        progress_log_file=progress_log_file,
    )
    
    # Add gradient norm tracking callback
    class GradientNormCallback:
        def __init__(self, trainer_instance):
            self.trainer = trainer_instance
        
        def on_step_end(self, args, state, control, **kwargs):
            # Compute gradient norm after backward pass
            model = kwargs.get('model')
            if model is not None:
                total_norm = 0.0
                param_count = 0
                for p in model.parameters():
                    if p.grad is not None:
                        param_norm = p.grad.data.norm(2)
                        total_norm += param_norm.item() ** 2
                        param_count += 1
                total_norm = total_norm ** 0.5
                
                self.trainer.gradient_norm_history.append({
                    'step': state.global_step,
                    'total_norm': total_norm,
                    'param_count': param_count,
                })
                
                # Keep only last 1000 steps
                if len(self.trainer.gradient_norm_history) > 1000:
                    self.trainer.gradient_norm_history.pop(0)
            
            return control
    
    # Note: Transformers Trainer doesn't support custom callbacks easily
    # Gradient norm will be logged via Trainer's built-in logging
    
    # Override trainer's dataloader with trajectory sampler (DDP-sharded)
    world_size = training_args.world_size if is_distributed else 1
    def get_train_dataloader_with_sampler():
        from torch.utils.data import DataLoader
        # Shard trajectories by rank so each GPU processes 1/world_size of data
        all_trajs = list(train_sampler.trajectories)
        rank_trajs = all_trajs[rank::world_size]
        rank_traj_indices = {t: train_sampler.traj_to_indices[t] for t in rank_trajs}
        rank_sampler = TrajectoryBatchSampler.__new__(TrajectoryBatchSampler)
        rank_sampler.traj_to_indices = rank_traj_indices
        rank_sampler.trajectories = rank_trajs
        rank_sampler.batch_size = train_sampler.batch_size
        rank_sampler.shuffle = True
        print(f"Rank {rank}: {len(rank_trajs):,} trajectories ({len(rank_trajs)/len(all_trajs)*100:.1f}% of total)")
        return DataLoader(
            train_dataset,
            batch_sampler=rank_sampler,
            collate_fn=collate_fn(tokenizer),
            num_workers=4,
            prefetch_factor=2,
            pin_memory=training_args.dataloader_pin_memory,
        )
    trainer.get_train_dataloader = get_train_dataloader_with_sampler
    
    # Train
    print("\nStarting training...")
    resume_checkpoint = args.resume_from_checkpoint
    if resume_checkpoint is None:
        output_path = Path(config['output_dir'])
        if output_path.exists():
            checkpoints = sorted(output_path.glob('checkpoint-*'), key=lambda x: int(x.name.split('-')[1]))
            valid_checkpoints = [cp for cp in checkpoints if (cp / 'trainer_state.json').exists()]
            if valid_checkpoints:
                resume_checkpoint = str(valid_checkpoints[-1])
                print(f"🔄 Auto-detected checkpoint: {resume_checkpoint}")

    trainer.train(resume_from_checkpoint=resume_checkpoint)
    
    # Save
    print("\nSaving final model...")
    trainer.save_model(f"{config['output_dir']}/final_model")
    
    print("\n" + "="*70)
    print("✅ TRAINING COMPLETE")
    print("="*70)

if __name__ == "__main__":
    main()

