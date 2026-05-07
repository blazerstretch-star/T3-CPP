#!/bin/bash

# EVAL ONLY: No-LDCA Model - Standard Evaluation

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/outputs_simple_labels_no_ldca"

cd ${SCRIPT_DIR}
source ${SCRIPT_DIR}/prm_env/bin/activate

CHECKPOINT=$(ls -d ${OUTPUT_DIR}/checkpoint-* 2>/dev/null | sort -V | tail -1)

if [ -z "$CHECKPOINT" ]; then
    echo "❌ No checkpoints found in ${OUTPUT_DIR}"
    exit 1
fi

echo "=========================================="
echo "EVAL ONLY: NO-LDCA MODEL"
echo "=========================================="
echo "Checkpoint: ${CHECKPOINT}"

python train_prm_equal_weights_no_ldca.py \
    --eval_only \
    --eval_checkpoint ${CHECKPOINT} \
    --base_model Qwen/Qwen2.5-Coder-1.5B \
    --max_length 6144 \
    --batch_size 16 \
    --use_arrow

echo "Results saved to: ${OUTPUT_DIR}/eval_results_no_ldca_$(basename ${CHECKPOINT}).json"
