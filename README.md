# T3-CPP Bench: A Repository-Level C++ Benchmark and a Stratified Process-Reward Framework

This repository is the official implementation of **T3-CPP Bench: A Repository-Level C++ Benchmark and a Stratified Process-Reward Framework**.

---

## Overview

T3-CPP Bench is a repository-level C++ benchmark for evaluating LLM code generation agents, paired with a step-level Process Reward Model (PRM) training framework using:
- **Monte Carlo Tree Search (MCTS)** for trajectory collection across 21 C++ repositories
- **Hybrid MC + LLM-as-a-Judge** step labeling
- **LDCA (Luck-Decoupled Credit Assignment)** — asymmetric loss weighting to handle trajectory-level luck

---

## Dataset

The dataset is hosted on Kaggle:

**[T3-CPP Bench Dataset](https://kaggle.com/datasets/4f5b21dbeeb2ccadc8cfe6fca9c5c2e326e5c49af61aba957ec746f0a05faf3d)**

| Split | Steps | Trajectories | Repos |
|-------|-------|-------------|-------|
| Train | 862,818 | 100,094 | 16 |
| Val | 61,316 | 6,301 | 7 |
| Test | 200,186 | 22,574 | 5 (held-out, zero-shot) |
| **Total** | **1,124,320** | **128,969** | **21** |

Download and place under `data/`:
```
data/
  train.jsonl
  val.jsonl
  test.jsonl
  merged.jsonl
```

---

## Setup

```bash
bash setup.sh
```

This will:
1. Clone the 3 excluded benchmark repositories into `benchmarks/` (see License section)
2. Install Python dependencies from `requirements.txt`

---

## Benchmark

The benchmark evaluates LLM agents on repository-level C++ function completion tasks. Each task has a function stub, unit tests, and a Docker environment for isolated compilation and testing.

### 4 Evaluation Settings

| Setting | Script | Description |
|---------|--------|-------------|
| `single_pass` | `run_api_single_pass.py` | Zero-shot, one prompt, no tools |
| `retrieval` | `run_api_single_pass.py --use_pregenerated_prompts` | Single-pass with BM25+dense retrieved context |
| `oracle` | `run_api_single_pass.py --use_pregenerated_prompts` | Single-pass with ground-truth oracle context |
| `agentic` | `run_api_agent.py` | Multi-turn agent with tool use (read files, search, compile) |

### Environment Variables

```bash
export INFERENCE_API_KEY="your-api-key"
export INFERENCE_PROVIDER="https://integrate.api.nvidia.com/v1/chat/completions"
# or: https://api.openai.com/v1/chat/completions
# or: http://localhost:8000/v1/chat/completions  (vLLM)
```

### Run Single-Pass (zero-shot)

```bash
python run_api_single_pass.py \
  --model qwen/qwen3.5-397b-a17b \
  --repos all \
  --setting single_pass \
  --api_key $INFERENCE_API_KEY \
  --temperature 0.0
```

### Run Retrieval-Augmented

First generate retrieval prompts (or use pre-generated ones in `ablation_prompts_retrieval/`):
```bash
# Use pre-generated prompts
python run_api_single_pass.py \
  --model qwen/qwen3.5-397b-a17b \
  --repos all \
  --setting retrieval \
  --use_pregenerated_prompts \
  --prompts_path "ablation_prompts_retrieval/{repo}/retrieval/prompts.jsonl" \
  --api_key $INFERENCE_API_KEY

# Or generate fresh retrieval prompts
python scripts/generate_prompts_with_retrieval.py --repo leveldb
```

### Run Oracle

```bash
# Use pre-generated oracle prompts
python run_api_single_pass.py \
  --model qwen/qwen3.5-397b-a17b \
  --repos all \
  --setting oracle \
  --use_pregenerated_prompts \
  --prompts_path "ablation_prompts_oracle/oracle/{repo}/baseline_oracle/prompts.jsonl" \
  --api_key $INFERENCE_API_KEY

# Or generate fresh oracle prompts
python scripts/generate_prompts_with_oracle.py --repo leveldb
```

### Run Agentic

```bash
python run_api_agent.py \
  --model qwen/qwen3.5-397b-a17b \
  --repos all \
  --setting agentic \
  --api_key $INFERENCE_API_KEY \
  --max_iterations 12 \
  --max_tool_calls 36 \
  --max_tokens 100000 \
  --beam_width 1
```

### Multiple API Keys (parallel repos)

```bash
python run_api_agent.py \
  --model qwen/qwen3.5-397b-a17b \
  --repos all \
  --api_keys "key1,key2,key3" \
  --setting agentic
```

### Resume interrupted runs

```bash
python run_api_agent.py --repos all --resume --api_key $INFERENCE_API_KEY
```

---

## Core Infrastructure (`core/`)

| File | Description |
|------|-------------|
| `agentic_tools.py` | Tool definitions exposed to the agent: `read_files`, `search_repo_code`, `search_files`, `list_directory`, `test_code_snippet` (compiles in Docker) |
| `agentic_strategies.py` | Budget strategies: `iteration` (max steps), `token` (max tokens), `time` (max seconds), `hybrid` (all three) |
| `trajectory_builder.py` | Serializes agent trajectories to `.json` files with step-level state, action, observation, reasoning, and token counts |
| `robust_injector.py` | Injects generated C++ code into the source file and runs tests inside Docker — returns `compiles`, `tests_pass`, `full_test_results` |
| `hybrid_retriever_vllm.py` | BM25 + dense embedding retrieval for context augmentation (retrieval setting) |
| `cpp_docker_pass_k_fixed.py` | Pass@k evaluation across beam search results |
| `llm_judge_framework/` | LLM-as-a-judge scoring pipeline used during MCTS labeling |

---

## Pre-generated Prompts

### `ablation_prompts_retrieval/`
Contains pre-generated retrieval-augmented prompts for all 21 repos:
```
ablation_prompts_retrieval/<repo>/retrieval/prompts.jsonl
```
Each line: `{"namespace": "...", "prompt": "..."}` — the full prompt with BM25+dense retrieved context injected.

### `ablation_prompts_oracle/`
Contains pre-generated oracle prompts (ground-truth context):
```
ablation_prompts_oracle/oracle/<repo>/baseline_oracle/
```
Used for the oracle upper-bound setting.

---

## Adding Excluded Repositories

Due to licensing restrictions, 3 repositories are excluded from the public release but their benchmark tasks are included. `setup.sh` clones them automatically, or manually:

```bash
git clone https://github.com/AzkaSahar/hospital-management-system benchmarks/hospital/context/hospital-management-system
git clone https://github.com/FaizaanAlFaisal/Movie-Recommendation-System benchmarks/movie_recommendation/context/Movie-Recommendation-System
git clone https://github.com/natrux/suffixtree benchmarks/suffixtree/context/suffixtree
```

---

## Pre-trained Models

Checkpoints are hosted on HuggingFace:

**[blz-stretch/Checkpoints_T3_CPP](https://huggingface.co/blz-stretch/Checkpoints_T3_CPP)**

| Model | Description |
|-------|-------------|
| `no_ldca/checkpoint-9000` | PRM without LDCA (uniform loss weighting) |
| `ldca/checkpoint-9000` | PRM with Strong LDCA (alpha_victim=2.0, alpha_lucky=1.5) |

Both are LoRA adapters (r=16, alpha=32) on `Qwen/Qwen2.5-Coder-1.5B`.

Place under:
```
MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE/checkpoints/
```

---

## Requirements

```bash
pip install -r requirements.txt
```

Key dependencies: `torch==2.6.0+cu124`, `transformers==5.5.4`, `peft==0.19.1`, `accelerate==1.13.0`, `flash_attn==2.8.3`, `orjson==3.11.8`

---

## Training

### PRM with LDCA (main model):
```bash
cd MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE
bash run_simple_labels_strong_ldca_training.sh
```

Or directly:
```bash
torchrun --nproc_per_node=3 --master_port=29502 \
  train_adaptive_prm_equal_weights.py \
  --base_model Qwen/Qwen2.5-Coder-1.5B \
  --max_length 6144 --batch_size 16 --grad_accum 2 \
  --use_arrow --epochs 1 --alpha_victim 2.0 --alpha_lucky 1.5
```

### PRM without LDCA (ablation):
```bash
bash run_simple_labels_no_ldca_training.sh
```

---

## Evaluation

```bash
cd MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE

# Val set
bash run_eval_ldca.sh
bash run_eval_no_ldca.sh

# Test set
bash run_simple_labels_strong_ldca_eval_test.sh
bash run_simple_labels_no_ldca_eval_test.sh

# Base model (untrained baseline)
bash run_base_model_eval_test.sh

# Cross-model trajectory pooling
bash run_cross_model_eval.sh <checkpoint> mean
```

---

## Results

### Per-Model Pass@1 Across Settings (331 problems)

Full trajectory files will be released at: `Bench_Results_Trajectories_Nips/`

| Model | Agentic | Oracle | Retrieval | Single-Pass | Avg |
|-------|---------|--------|-----------|-------------|-----|
| moonshotai/kimi-k2-instruct | 72.81% | 50.45% | 51.36% | 37.76% | **53.10%** |
| mistralai/mistral-large-3-675b | 67.37% | 47.43% | 49.85% | 35.95% | **50.15%** |
| google/gemma-4-31b-it | 73.41% | 47.43% | 44.71% | 34.14% | **49.92%** |
| qwen/qwen3-next-80b-a3b-instruct | 68.88% | 44.41% | 44.11% | 32.93% | **47.58%** |
| qwen/qwen3-coder-480b-a35b-instruct | 58.31% | 44.71% | 44.41% | 36.86% | **46.07%** |
| openai/gpt-oss-120b | 21.45% | 57.10% | 52.27% | 42.30% | **43.28%** |
| minimaxai/minimax-m2.7 | 51.36% | 44.11% | 41.39% | 33.23% | **42.52%** |
| nvidia/nemotron-3-nano-30b-a3b | 57.40% | 38.37% | 38.07% | 24.77% | **39.65%** |
| meta/llama-3.3-70b-instruct | 52.27% | 38.07% | 39.58% | 29.31% | **39.80%** |
| mistralai/ministral-14b-instruct | 64.05% | 36.25% | 34.74% | 24.17% | **39.80%** |
| stepfun-ai/step-3.5-flash | 56.50% | — | — | — | **56.50%** |

> Note: stepfun-ai evaluated on agentic setting only. Oracle/retrieval/single-pass results excluded due to weak performance.

### Cross-Model Trajectory Pooling (Test Set, 66 tasks)

| Method | Pass@1 |
|--------|--------|
| Oracle (upper bound) | 0.9848 (65/66) |
| Best single-model (qwen3.5-397b) | 1.0000 (3/3) |
| **Cross-model PRM (LDCA)** | **0.9697 (64/66)** |
| **Cross-model PRM (no LDCA)** | **0.9697 (64/66)** |

### Benchmark Pass Rates (Agentic Setting, averaged across models)

| Repo | Pass Rate |
|------|-----------|
| tinyrenderer | 78.2% |
| numerical_methods | 77.8% |
| cgal_kernel | 76.5% |
| cpp-projects | 75.3% |
| molecular_dynamics | 74.5% |

Full per-model, per-repo, per-problem results: `Bench_Results_Trajectories_Nips/results.json`

---

## Repository Structure

```
.
├── README.md
├── requirements.txt
├── setup.sh                              # Clone excluded repos + install deps
│
├── run_api_agent.py                      # Agentic multi-turn inference (tool use, MCTS)
├── run_api_single_pass.py                # Single-pass / retrieval / oracle inference
│
├── benchmarks/                           # 24 C++ repos with tasks, tests, Dockerfiles
│   └── <repo>/
│       ├── context/                      # Repo source code (read by agent)
│       ├── src/                          # Function stubs + complete implementations
│       ├── tests/                        # Unit tests (compiled + run in Docker)
│       ├── metadata.jsonl                # Task definitions
│       ├── Dockerfile
│       └── run_tests.sh
│
├── core/                                 # Shared infrastructure
│   ├── agentic_tools.py                  # Tool definitions (read, search, compile)
│   ├── agentic_strategies.py             # Budget strategies
│   ├── trajectory_builder.py             # Trajectory serialization
│   ├── robust_injector.py                # Docker code injection + test runner
│   ├── hybrid_retriever_vllm.py          # BM25 + dense retrieval
│   ├── cpp_docker_pass_k_fixed.py        # Pass@k evaluation
│   └── llm_judge_framework/              # LLM-as-a-judge scoring
│
├── scripts/
│   ├── generate_prompts_with_retrieval.py
│   ├── generate_prompts_with_oracle.py
│   └── generate_all_retrieval_prompts.py
│
├── ablation_prompts_retrieval/           # Pre-generated retrieval prompts
│   └── <repo>/retrieval/prompts.jsonl
│
├── ablation_prompts_oracle/              # Pre-generated oracle prompts
│   └── oracle/<repo>/baseline_oracle/
│
├── MCTS_LABELED_FINAL_TUE_02_20_PM/
│   ├── llm_as_a_judge_framework/
│   │   ├── raw_trajectory/               # Raw MCTS trajectories (on Kaggle)
│   │   └── step_wise_data/               # Labeled splits: train/val/test/merged
│   └── Scripts/training/PRM_JUDGE/       # PRM training + evaluation
│       ├── train_adaptive_prm_equal_weights.py
│       ├── train_prm_equal_weights_no_ldca.py
│       ├── eval_prm.py
│       ├── eval_cross_model_pooling.py
│       └── run_*.sh
│
└── data/                                 # Download from Kaggle
    ├── train.jsonl
    ├── val.jsonl
    ├── test.jsonl
    └── merged.jsonl
```

---

## Dataset License Compliance Notice

The full dataset comprised **135,259 trajectories** and **1,189,367 labeled steps** across 23 C++ repositories.

Due to licensing restrictions, the following repositories **cannot be redistributed**:

| Repository | Removed Raw Trajectories | Removed Steps | Split Affected |
|------------|--------------------------|---------------|----------------|
| `movie_recommendation` | 2,453 | 25,745 | Train |
| `hospital` | 3,867 | 39,302 | Test |
| `suffixtree` | 660 | — | Bench results only |
| **Total** | **6,320** | **65,047** | — |

**Publicly available on Kaggle:**

| Split | Steps | Raw Trajectories |
|-------|-------|------------------|
| Train | 862,818 | — |
| Val | 61,316 | — |
| Test | 200,186 | — |
| **Total** | **1,124,320** | **129,770** |

Benchmark pass rate results (`results.json`) retain entries for all repositories including removed ones — these are evaluation metrics only, no source code is redistributed.

To reproduce results on excluded repositories, clone them from GitHub (see **Adding Excluded Repositories** above).

---

## License

Code: MIT License

Dataset: CC BY 4.0
