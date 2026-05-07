#!/bin/bash

# Cross-model pooling evaluation
# Usage: ./run_cross_model_eval.sh <checkpoint> [aggregation] [max_samples] [num_gpus]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BASE_OUTPUT_DIR="${SCRIPT_DIR}/Ablation/Cross_Model"

CHECKPOINT="${1:-${SCRIPT_DIR}/outputs_simple_labels_no_ldca/checkpoint-9000}"
AGGREGATION="${2:-mean}"
MAX_SAMPLES="${3:-}"
NUM_GPUS="${4:-1}"

if [[ "$AGGREGATION" == "last" ]]; then
    TEST_CACHE="${SCRIPT_DIR}/../../../../data/test_tokenized_Qwen2.5-Coder-1.5B_maxlen6144_eval.arrow"
else
    TEST_CACHE="${SCRIPT_DIR}/../../../../data/test_tokenized_Qwen2.5-Coder-1.5B_maxlen6144_eval_prefixes.arrow"
fi

TEST_FILE="${SCRIPT_DIR}/../../../../data/test.jsonl"

if [[ "$CHECKPOINT" == "baseline" ]]; then
    RESULT_FOLDER="baseline"
else
    PARENT=$(basename $(dirname "$CHECKPOINT"))
    CKPT=$(basename "$CHECKPOINT")
    RESULT_FOLDER="${PARENT}_${CKPT}"
fi

OUTPUT_DIR="${BASE_OUTPUT_DIR}/${RESULT_FOLDER}_strategy_${AGGREGATION}"
mkdir -p "$OUTPUT_DIR"

echo "=================================="
echo "Cross-Model Pooling Evaluation"
echo "=================================="
echo "Checkpoint:  $CHECKPOINT"
echo "Aggregation: $AGGREGATION"
echo "Output:      $OUTPUT_DIR"
echo ""

SESSION_NAME="prm_eval_${AGGREGATION}_$(basename $RESULT_FOLDER)"

if [ "$NUM_GPUS" -eq 1 ]; then
    screen -dmS "$SESSION_NAME" bash -c "
        cd '$SCRIPT_DIR' && \
        source '${SCRIPT_DIR}/prm_env/bin/activate' && \
        export CUDA_VISIBLE_DEVICES=0 && \
        export PYTHONUNBUFFERED=1 && \
        python -u eval_cross_model_pooling.py \
            --checkpoint '$CHECKPOINT' \
            --test_file '$TEST_FILE' \
            --output_dir '$OUTPUT_DIR' \
            --batch_size 64 \
            --aggregation '$AGGREGATION' \
            $([ -n '$MAX_SAMPLES' ] && echo '--max_samples $MAX_SAMPLES') \
            2>&1 | tee '$OUTPUT_DIR/evaluation.log'
    "
else
    screen -dmS "$SESSION_NAME" bash -c "
        cd '$SCRIPT_DIR' && \
        source '${SCRIPT_DIR}/prm_env/bin/activate' && \
        export PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True,max_split_size_mb:512 && \
        export CUDA_VISIBLE_DEVICES=0,1,2 && \
        export NCCL_TIMEOUT=3600 && \
        export TORCH_NCCL_ASYNC_ERROR_HANDLING=1 && \
        export TORCH_NCCL_BLOCKING_WAIT=1 && \
        export NCCL_IB_DISABLE=1 && \
        export NCCL_SOCKET_IFNAME=^lo,docker,virbr0 && \
        export OMP_NUM_THREADS=8 && \
        export PYTHONUNBUFFERED=1 && \
        export TOKENIZERS_PARALLELISM=false && \
        stdbuf -oL -eL torchrun --nproc_per_node=$NUM_GPUS --master_port=29503 eval_cross_model_pooling.py \
            --checkpoint '$CHECKPOINT' \
            --test_file '$TEST_FILE' \
            --output_dir '$OUTPUT_DIR' \
            --batch_size 64 \
            --aggregation '$AGGREGATION' \
            --multi_gpu \
            $([ -n '$MAX_SAMPLES' ] && echo '--max_samples $MAX_SAMPLES') \
            2>&1 | tee '$OUTPUT_DIR/evaluation.log'
    "
fi

echo "✅ Started | Screen: $SESSION_NAME | Log: $OUTPUT_DIR/evaluation.log"
echo "Monitor: screen -r $SESSION_NAME"
