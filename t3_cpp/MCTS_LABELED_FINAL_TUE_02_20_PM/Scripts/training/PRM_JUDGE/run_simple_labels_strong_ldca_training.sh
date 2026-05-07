#!/bin/bash

# SIMPLE LABELS + STRONG LDCA PRM Training
# 50-50 label blending + Strong LDCA (alpha_victim=2.0, alpha_lucky=1.5)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/outputs_simple_labels_strong_ldca"
LOG_FILE="${OUTPUT_DIR}/training.log"
SCREEN_NAME="prm_simple_strong_ldca"
DATA_DIR="${SCRIPT_DIR}/../../../../data"

mkdir -p "${OUTPUT_DIR}"

screen -dmS "${SCREEN_NAME}" bash -c "
    source ${SCRIPT_DIR}/prm_env/bin/activate && \
    cd ${SCRIPT_DIR} && \
    
    export PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True,max_split_size_mb:512 && \
    export CUDA_VISIBLE_DEVICES=0,1,2 && \
    export CUDA_LAUNCH_BLOCKING=0 && \
    export NCCL_TIMEOUT=3600 && \
    export TORCH_NCCL_ASYNC_ERROR_HANDLING=1 && \
    export TORCH_NCCL_BLOCKING_WAIT=1 && \
    export NCCL_DEBUG=WARN && \
    export NCCL_IB_DISABLE=1 && \
    export NCCL_P2P_DISABLE=0 && \
    export NCCL_SOCKET_IFNAME=^lo,docker,virbr0 && \
    export NCCL_MIN_NRINGS=4 && \
    export TORCH_NCCL_HEARTBEAT_TIMEOUT_SEC=3600 && \
    export TORCH_NCCL_ENABLE_MONITORING=1 && \
    export OMP_NUM_THREADS=8 && \
    export MKL_NUM_THREADS=8 && \
    export PYTHONUNBUFFERED=1 && \
    export TOKENIZERS_PARALLELISM=false && \
    
    stdbuf -oL -eL torchrun --nproc_per_node=3 --master_port=29502 train_adaptive_prm_equal_weights.py \
        --base_model Qwen/Qwen2.5-Coder-1.5B \
        --max_length 6144 \
        --batch_size 16 \
        --grad_accum 2 \
        --use_arrow \
        --epochs 1 \
        --alpha_victim 2.0 \
        --alpha_lucky 1.5 2>&1 | tee ${LOG_FILE}
"

echo "=========================================="
echo "SIMPLE LABELS + STRONG LDCA TRAINING STARTED"
echo "=========================================="
echo "Screen: ${SCREEN_NAME} | Log: ${LOG_FILE}"
echo "Monitor: screen -r ${SCREEN_NAME}"
echo "=========================================="
