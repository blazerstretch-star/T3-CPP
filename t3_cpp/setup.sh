#!/bin/bash
# setup.sh
# Clones the 3 excluded benchmark repositories (hospital, movie_recommendation, suffixtree)
# into their respective context/ folders, then sets up the Python environment.
#
# These repos are excluded from the public dataset release due to licensing restrictions.
# See README.md (Dataset License Compliance Notice) for details.

set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCHMARKS_DIR="${REPO_ROOT}/benchmarks"

echo "=================================================="
echo "T3-CPP Bench Setup"
echo "=================================================="
echo ""

# ── 1. Clone excluded benchmark repositories ──────────────────────────────────
echo "Step 1: Cloning excluded benchmark repositories..."
echo ""

clone_if_empty() {
    local repo_url="$1"
    local dest="$2"
    local name="$3"

    if [ -d "$dest" ] && [ "$(ls -A $dest)" ]; then
        echo "  ✅ $name already present, skipping"
    else
        echo "  Cloning $name..."
        mkdir -p "$dest"
        git clone --depth=1 "$repo_url" "$dest"
        echo "  ✅ $name cloned"
    fi
}

clone_if_empty \
    "https://github.com/AzkaSahar/hospital-management-system" \
    "${BENCHMARKS_DIR}/hospital/context/hospital-management-system" \
    "hospital"

clone_if_empty \
    "https://github.com/FaizaanAlFaisal/Movie-Recommendation-System" \
    "${BENCHMARKS_DIR}/movie_recommendation/context/Movie-Recommendation-System" \
    "movie_recommendation"

clone_if_empty \
    "https://github.com/natrux/suffixtree" \
    "${BENCHMARKS_DIR}/suffixtree/context/suffixtree" \
    "suffixtree"

echo ""
echo "Step 2: Installing Python dependencies..."
echo ""

TRAINING_DIR="${REPO_ROOT}/MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE"

if [ -f "${TRAINING_DIR}/requirements_main.txt" ]; then
    pip install -r "${TRAINING_DIR}/requirements_main.txt"
    echo "  ✅ Dependencies installed"
else
    echo "  ⚠️  requirements_main.txt not found at ${TRAINING_DIR}"
    echo "     Run: pip install torch transformers peft accelerate orjson tqdm datasets safetensors scipy"
fi

echo ""
echo "=================================================="
echo "✅ Setup complete!"
echo "=================================================="
echo ""
echo "Next steps:"
echo "  1. Download dataset from Kaggle and place in data/"
echo "     https://kaggle.com/datasets/4f5b21dbeeb2ccadc8cfe6fca9c5c2e326e5c49af61aba957ec746f0a05faf3d"
echo ""
echo "  2. Download checkpoints from HuggingFace:"
echo "     https://huggingface.co/blz-stretch/Checkpoints_T3_CPP"
echo "     Place under: MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE/checkpoints/"
echo ""
echo "  3. Train:"
echo "     cd MCTS_LABELED_FINAL_TUE_02_20_PM/Scripts/training/PRM_JUDGE"
echo "     bash run_simple_labels_strong_ldca_training.sh"
echo ""
echo "  4. Evaluate:"
echo "     bash run_simple_labels_strong_ldca_eval_test.sh"
echo "=================================================="
