#!/bin/bash
# Pre-tokenize test data in screen with logging
# Usage: 
#   ./run_pretokenize_venv.sh full      # For 'last' aggregation (default, fast)
#   ./run_pretokenize_venv.sh prefixes  # For 'mean'/'min'/'max' aggregations (slower)

set -e

# Parse mode argument
MODE="${1:-full}"  # Default to 'full' if not specified

if [[ "$MODE" != "full" && "$MODE" != "prefixes" ]]; then
    echo "❌ ERROR: Invalid mode '$MODE'"
    echo "Usage: $0 [full|prefixes]"
    echo "  full:     Cache full trajectories (for 'last' aggregation) - FAST"
    echo "  prefixes: Cache all step prefixes (for 'mean'/'min'/'max') - SLOW"
    exit 1
fi

# Configuration
BASE_MODEL="Qwen/Qwen2.5-Coder-1.5B"
MAX_LENGTH=6144
TEST_FILE="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl"
LOG_FILE="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test_tokenize.log"
SCREEN_NAME="test_tokenize"
CUDA_DEVICE="3"
VENV_PATH="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE/prm_env"

# Extract model name for cache filename
MODEL_NAME=$(basename $BASE_MODEL)
if [ "$MODE" = "full" ]; then
    CACHE_FILE="${TEST_FILE%.jsonl}_tokenized_${MODEL_NAME}_maxlen${MAX_LENGTH}_eval.arrow"
    MODE_DESC="FULL trajectories (for 'last' aggregation)"
    TIME_EST="~1-2 minutes for 26k trajectories"
else
    CACHE_FILE="${TEST_FILE%.jsonl}_tokenized_${MODEL_NAME}_maxlen${MAX_LENGTH}_eval_prefixes.arrow"
    MODE_DESC="ALL step prefixes (for 'mean'/'min'/'max' aggregations)"
    TIME_EST="~10-15 minutes for ~200k prefixes"
fi

echo "=========================================="
echo "PRE-TOKENIZATION CONFIGURATION"
echo "=========================================="
echo "Mode: $MODE"
echo "  → $MODE_DESC"
echo "Base model: $BASE_MODEL"
echo "Max length: $MAX_LENGTH"
echo "CUDA device: $CUDA_DEVICE"
echo "Virtual env: $VENV_PATH"
echo "Test file: $TEST_FILE"
echo "Cache file: $CACHE_FILE"
echo "Log file: $LOG_FILE"
echo "Screen name: $SCREEN_NAME"
echo "Estimated time: $TIME_EST"
echo "=========================================="

# Check if venv exists
if [ ! -f "$VENV_PATH/bin/activate" ]; then
    echo ""
    echo "❌ ERROR: Virtual environment not found!"
    echo "   Expected: $VENV_PATH/bin/activate"
    exit 1
fi

# Check if cache already exists
if [ -d "$CACHE_FILE" ]; then
    echo ""
    echo "⚠️  WARNING: Cache already exists!"
    echo "   $CACHE_FILE"
    echo ""
    read -p "Do you want to overwrite? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
    echo "Removing existing cache..."
    rm -rf "$CACHE_FILE"
fi

# Check if screen session already exists
if screen -list | grep -q "$SCREEN_NAME"; then
    echo ""
    echo "⚠️  Screen session '$SCREEN_NAME' already exists!"
    echo "   Attach with: screen -r $SCREEN_NAME"
    echo "   Or kill with: screen -X -S $SCREEN_NAME quit"
    exit 1
fi

# Create screen command with unbuffered output
screen -dmS $SCREEN_NAME -L -Logfile $LOG_FILE bash -c "
set -e
export PYTHONUNBUFFERED=1
export CUDA_VISIBLE_DEVICES=$CUDA_DEVICE

# Activate virtual environment
source $VENV_PATH/bin/activate

echo '========================================='
echo 'PRE-TOKENIZATION STARTED'
echo '========================================='
echo \"Timestamp: \$(date)\"
echo 'Mode: $MODE'
echo 'Base model: $BASE_MODEL'
echo 'Max length: $MAX_LENGTH'
echo \"CUDA device: \$CUDA_VISIBLE_DEVICES\"
echo \"Python: \$(which python)\"
echo 'Virtual env: $VENV_PATH'
echo 'Test file: $TEST_FILE'
echo 'Cache file: $CACHE_FILE'
echo '========================================='
echo ''

python -u pretokenize_test_data.py \\
    --test_file '$TEST_FILE' \\
    --base_model '$BASE_MODEL' \\
    --max_length $MAX_LENGTH \\
    --mode '$MODE'

EXIT_CODE=\$?

echo ''
echo '========================================='
echo 'PRE-TOKENIZATION FINISHED'
echo '========================================='
echo \"Timestamp: \$(date)\"
echo \"Exit code: \$EXIT_CODE\"
echo 'Cache file: $CACHE_FILE'

if [ \$EXIT_CODE -eq 0 ]; then
    if [ -d '$CACHE_FILE' ]; then
        CACHE_SIZE=\$(du -sh '$CACHE_FILE' | cut -f1)
        echo \"Cache size: \$CACHE_SIZE\"
        echo '✅ SUCCESS: Pre-tokenization complete!'
    else
        echo '❌ ERROR: Cache file not created!'
    fi
else
    echo \"❌ ERROR: Pre-tokenization failed with exit code \$EXIT_CODE\"
fi
echo '========================================='

# Keep screen alive for 10 seconds so you can see the final message
sleep 10
"

echo ""
echo "=========================================="
echo "✅ SCREEN SESSION STARTED"
echo "=========================================="
echo "Mode: $MODE ($MODE_DESC)"
echo "Screen name: $SCREEN_NAME"
echo "CUDA device: $CUDA_DEVICE"
echo "Virtual env: $VENV_PATH"
echo "Log file: $LOG_FILE"
echo ""
echo "Commands:"
echo "  Attach to screen:  screen -r $SCREEN_NAME"
echo "  Detach from screen: Ctrl+A, then D"
echo "  View log (live):   tail -f $LOG_FILE"
echo "  Kill screen:       screen -X -S $SCREEN_NAME quit"
echo "=========================================="
echo ""
echo "⏳ Pre-tokenization running in background..."
echo "   $TIME_EST"
echo ""
