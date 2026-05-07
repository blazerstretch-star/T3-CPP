#!/bin/bash
# Run baseline (untrained) model evaluation for comparison

set -e

echo "=========================================="
echo "Baseline (Untrained) Model Evaluation"
echo "=========================================="
echo ""

# Configuration
BASE_DIR="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE"
TEST_FILE="/mnt/disk1/LLMforCode/test/aditikanishk/EvoCodeBench-CPP/MCTS_LABELED_FINAL_TUE_02_20_PM/llm_as_a_judge_framework/clipped_mc_soft_judged_split_train/test.jsonl"
OUTPUT_DIR="${BASE_DIR}/Ablation/Baseline_Untrained"

echo "Test file: $TEST_FILE"
echo "Output: $OUTPUT_DIR"
echo ""
echo "NOTE: This evaluates an UNTRAINED model (random initialization)"
echo "      to show the value of training vs random selection."
echo ""

# Run evaluation
python eval_cross_model_pooling.py \
    --checkpoint baseline \
    --test_file "$TEST_FILE" \
    --output_dir "$OUTPUT_DIR" \
    2>&1 | tee "$OUTPUT_DIR/baseline_evaluation.log"

echo ""
echo "=========================================="
echo "✅ Baseline evaluation complete!"
echo "=========================================="
echo "Results: $OUTPUT_DIR/cross_model_results.json"
echo "Log: $OUTPUT_DIR/baseline_evaluation.log"
echo ""
echo "Compare with trained model:"
echo "  Trained:  Ablation/Cross_Model/cross_model_results.json"
echo "  Baseline: Ablation/Baseline_Untrained/cross_model_results.json"
