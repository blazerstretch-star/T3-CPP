#!/bin/bash

# EVAL ONLY: LDCA Model - Stratified Evaluation

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/outputs_simple_labels_strong_ldca"
CHECKPOINT="${OUTPUT_DIR}/checkpoint-9000"

cd ${SCRIPT_DIR}
source ${SCRIPT_DIR}/prm_env/bin/activate

echo "=========================================="
echo "EVAL ONLY: LDCA MODEL"
echo "=========================================="
echo "Checkpoint: ${CHECKPOINT}"

python train_adaptive_prm_equal_weights.py \
    --eval_only \
    --eval_checkpoint ${CHECKPOINT} \
    --base_model Qwen/Qwen2.5-Coder-1.5B \
    --max_length 6144 \
    --batch_size 16 \
    --use_arrow \
    --alpha_victim 2.0 \
    --alpha_lucky 1.5

echo "Results saved to: ${OUTPUT_DIR}/stratified_eval_results_checkpoint-9000.json"
