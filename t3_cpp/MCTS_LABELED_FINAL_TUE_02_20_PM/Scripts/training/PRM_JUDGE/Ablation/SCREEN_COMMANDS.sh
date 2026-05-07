#!/bin/bash
# 4 Independent Screen Commands for PRM Evaluation
# Run each command in a separate terminal/screen session
# Uses GPUs 0, 1, 2 (GPU 3 is non-functional)

BASE_DIR="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE"
CHECKPOINT="${BASE_DIR}/outputs_adaptive_prm/checkpoint-5000"
TEST_FILE="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl"
ABLATION_DIR="${BASE_DIR}/Ablation"

echo "========================================================================"
echo "4 INDEPENDENT SCREEN COMMANDS FOR PRM EVALUATION"
echo "========================================================================"
echo "Copy and paste each command in a separate terminal"
echo "Each will run in a detached screen session with unbuffered logging"
echo "========================================================================"
echo ""

cat << 'EOF'

# ============================================================================
# COMMAND 1: Trained Model (GPU 0)
# ============================================================================
screen -dmS prm_trained bash -c "cd /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE && CUDA_VISIBLE_DEVICES=0 python -u eval_prm.py --exp trained --checkpoint outputs_adaptive_prm/checkpoint-5000 --test_file /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl --output_dir Ablation/1_trained --batch_size 8 2>&1 | tee Ablation/1_trained/run.log; exec bash"

# To monitor: screen -r prm_trained
# To detach: Ctrl+A then D


# ============================================================================
# COMMAND 2: Untrained Baseline (GPU 1)
# ============================================================================
screen -dmS prm_untrained bash -c "cd /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE && CUDA_VISIBLE_DEVICES=1 python -u eval_prm.py --exp untrained --test_file /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl --output_dir Ablation/2_untrained --batch_size 8 2>&1 | tee Ablation/2_untrained/run.log; exec bash"

# To monitor: screen -r prm_untrained
# To detach: Ctrl+A then D


# ============================================================================
# COMMAND 3: Label Ablation Trained (GPU 2)
# ============================================================================
screen -dmS prm_ablation bash -c "cd /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE && CUDA_VISIBLE_DEVICES=2 python -u eval_prm.py --exp ablation --checkpoint outputs_adaptive_prm/checkpoint-5000 --test_file /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl --output_dir Ablation/3_ablation_trained --batch_size 8 2>&1 | tee Ablation/3_ablation_trained/run.log; exec bash"

# To monitor: screen -r prm_ablation
# To detach: Ctrl+A then D


# ============================================================================
# COMMAND 4: Label Ablation Untrained (GPU 0 - reuse after Command 1 finishes)
# ============================================================================
screen -dmS prm_ablation_untrained bash -c "cd /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE && CUDA_VISIBLE_DEVICES=0 python -u eval_prm.py --exp ablation_untrained --test_file /mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl --output_dir Ablation/4_ablation_untrained --batch_size 8 2>&1 | tee Ablation/4_ablation_untrained/run.log; exec bash"

# To monitor: screen -r prm_ablation_untrained
# To detach: Ctrl+A then D


# ============================================================================
# MONITORING COMMANDS
# ============================================================================

# List all screen sessions:
screen -ls

# Attach to a specific session:
screen -r prm_trained
screen -r prm_untrained
screen -r prm_ablation
screen -r prm_ablation_untrained

# View logs in real-time:
tail -f Ablation/1_trained/run.log
tail -f Ablation/2_untrained/run.log
tail -f Ablation/3_ablation_trained/run.log
tail -f Ablation/4_ablation_untrained/run.log

# Kill a screen session:
screen -X -S prm_trained quit
screen -X -S prm_untrained quit
screen -X -S prm_ablation quit
screen -X -S prm_ablation_untrained quit


# ============================================================================
# RESULTS LOCATIONS
# ============================================================================

# After completion, results will be in:
# 1. Ablation/1_trained/trained_results.json
# 2. Ablation/2_untrained/untrained_results.json
# 3. Ablation/3_ablation_trained/ablation_results.json
# 4. Ablation/4_ablation_untrained/ablation_untrained_results.json

# Logs will be in:
# 1. Ablation/1_trained/run.log
# 2. Ablation/2_untrained/run.log
# 3. Ablation/3_ablation_trained/run.log
# 4. Ablation/4_ablation_untrained/run.log

EOF

echo ""
echo "========================================================================"
echo "NOTES:"
echo "========================================================================"
echo "- Commands 1, 2, 3 run on GPUs 0, 1, 2 simultaneously"
echo "- Command 4 reuses GPU 0 (run after Command 1 finishes, or use GPU 1/2)"
echo "- All logs are unbuffered (python -u) and saved to run.log files"
echo "- Use 'screen -r <name>' to attach and monitor progress"
echo "- Use 'tail -f Ablation/*/run.log' to watch all logs"
echo "========================================================================"
