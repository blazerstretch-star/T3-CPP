#!/bin/bash

# NO LDCA PRM Evaluation on VAL SET - DDP VERSION

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/outputs_simple_labels_no_ldca"
CHECKPOINT="${OUTPUT_DIR}/checkpoint-9000"
LOG_FILE="${OUTPUT_DIR}/eval_val.log"
SCREEN_NAME="prm_eval_no_ldca"

mkdir -p "${OUTPUT_DIR}"

screen -dmS "${SCREEN_NAME}" bash -c "
    source ${SCRIPT_DIR}/prm_env/bin/activate && \
    cd ${SCRIPT_DIR} && \
    export PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True,max_split_size_mb:512 && \
    export CUDA_VISIBLE_DEVICES=0,1,2 && \
    export NCCL_TIMEOUT=3600 && \
    export TORCH_NCCL_ASYNC_ERROR_HANDLING=1 && \
    export TORCH_NCCL_BLOCKING_WAIT=1 && \
    export NCCL_IB_DISABLE=1 && \
    export NCCL_SOCKET_IFNAME=^lo,docker,virbr0 && \
    export NCCL_MIN_NRINGS=4 && \
    export TORCH_NCCL_HEARTBEAT_TIMEOUT_SEC=3600 && \
    export OMP_NUM_THREADS=8 && \
    export PYTHONUNBUFFERED=1 && \
    export TOKENIZERS_PARALLELISM=false && \
    stdbuf -oL -eL torchrun --nproc_per_node=3 --master_port=29503 train_adaptive_prm_equal_weights.py \
        --base_model Qwen/Qwen2.5-Coder-1.5B \
        --max_length 6144 \
        --batch_size 16 \
        --use_arrow \
        --alpha_victim 2.0 \
        --alpha_lucky 1.5 \
        --eval_only \
        --eval_checkpoint ${CHECKPOINT} 2>&1 | tee ${LOG_FILE}
"

echo "NO-LDCA VAL EVAL STARTED | Screen: ${SCREEN_NAME} | Log: ${LOG_FILE}"
