#!/bin/bash

# Safe MC‑PRM Labeling on Pre‑Split Folders
# Runs labeler on RESULTS_SPLIT_NEW_MON_9_30_PM/{short,openai,medium,long}
# No data mutations — safe to run multiple times

set -e

echo "Starting safe MC‑PRM labeling on split folders..."
echo ""

# ─────────────────────────────────────────────────────────────
# Create required directories
# ─────────────────────────────────────────────────────────────

mkdir -p MCTS_LABELED_FINAL_TUE_02_20_PM
mkdir -p logs/mcts_logs

timestamp=$(date +"%Y%m%d_%H%M%S")

# ── SHORT GROUP ──────────────────────────────────────────────
echo "=== SHORT GROUP ==="
echo "bytedance + qwen3-next-80b-a3b-thinking"

python3 mc_prm_labeler_ml_fixed.py \
--results_dir RESULTS_SPLIT_NEW_MON_9_30_PM/short \
--output_dir MCTS_LABELED_FINAL_TUE_02_20_PM/short \
--workers 120 \
--soft_mc \
--min_rollouts 5 \
--laplace_alpha 0.5 \
--advantage_neutral_band 0.01 \
--compress_state \
2>&1 | tee logs/mcts_logs/short_${timestamp}.log

echo ""
echo "✅ SHORT group completed"
echo ""

# ── OPENAI GROUP ─────────────────────────────────────────────
echo "=== OPENAI GROUP ==="
echo "openai alone"

python3 mc_prm_labeler_ml_fixed.py \
--results_dir RESULTS_SPLIT_NEW_MON_9_30_PM/openai \
--output_dir MCTS_LABELED_FINAL_TUE_02_20_PM/openai \
--workers 120 \
--soft_mc \
--min_rollouts 5 \
--laplace_alpha 0.5 \
--advantage_neutral_band 0.01 \
--compress_state \
2>&1 | tee logs/mcts_logs/openai_${timestamp}.log

echo ""
echo "✅ OPENAI group completed"
echo ""

# ── MEDIUM GROUP ─────────────────────────────────────────────
echo "=== MEDIUM GROUP ==="
echo "meta + nvidia + qwen3-next-80b-a3b-instruct"

python3 mc_prm_labeler_ml_fixed.py \
--results_dir RESULTS_SPLIT_NEW_MON_9_30_PM/medium \
--output_dir MCTS_LABELED_FINAL_TUE_02_20_PM/medium \
--workers 120 \
--soft_mc \
--min_rollouts 5 \
--laplace_alpha 0.5 \
--advantage_neutral_band 0.01 \
--compress_state \
2>&1 | tee logs/mcts_logs/medium_${timestamp}.log

echo ""
echo "✅ MEDIUM group completed"
echo ""

# ── LONG GROUP ───────────────────────────────────────────────
echo "=== LONG GROUP ==="
echo "qwen3.5-122b-a10b + qwen3.5-397b-a17b"

python3 mc_prm_labeler_ml_fixed.py \
--results_dir RESULTS_SPLIT_NEW_MON_9_30_PM/long \
--output_dir MCTS_LABELED_FINAL_TUE_02_20_PM/long \
--workers 120 \
--soft_mc \
--min_rollouts 5 \
--laplace_alpha 0.5 \
--advantage_neutral_band 0.01 \
--compress_state \
2>&1 | tee logs/mcts_logs/long_${timestamp}.log

echo ""
echo "✅ LONG group completed"
echo ""

# ── SUMMARY ──────────────────────────────────────────────────
echo "════════════════════════════════════════════════════════════════"
echo "✅ All four MC‑PRM groups labeled successfully!"
echo "════════════════════════════════════════════════════════════════"
echo ""

echo "Output structure:"
echo " MCTS_LABELED_FINAL_TUE_02_20_PM/"
echo " ├── short/"
echo " ├── openai/"
echo " ├── medium/"
echo " └── long/"
echo ""

echo "Logs saved to:"
echo " logs/mcts_logs/"
echo ""

echo "Next steps:"
echo "1. Normalize each group's output"
echo "2. Merge all four groups"
echo "3. Run hybrid labeler on merged dataset"
echo ""