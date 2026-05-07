#!/usr/bin/env python3
"""
Shared constants and contracts for the t3_cpp PRM pipeline.
All modules import from here to ensure cross-module consistency.
"""

from enum import Enum
from typing import Tuple

# ── Scale contracts ──────────────────────────────────────────────────────────
MC_ADVANTAGE_RANGE: Tuple[float, float] = (-1.0, 1.0)   # Enforced by mc_prm_labeler_ml_fixed.py
JUDGE_SCORE_RANGE: Tuple[float, float] = (0.0, 1.0)      # Raw LLM output range
CONSENSUS_LABEL_RANGE: Tuple[float, float] = (-1.0, 1.0) # PRM training target

def normalize_to_range(value: float,
                        src_lo: float, src_hi: float,
                        dst_lo: float = -1.0, dst_hi: float = 1.0) -> float:
    """Linearly map value from [src_lo, src_hi] to [dst_lo, dst_hi]."""
    if src_hi == src_lo:
        return 0.0
    normalized = (value - src_lo) / (src_hi - src_lo)
    return dst_lo + normalized * (dst_hi - dst_lo)

def normalize_to_prm_range(value: float, source_range: Tuple[float, float]) -> float:
    """Normalize value from source_range to PRM training range [-1, 1]."""
    return normalize_to_range(value, source_range[0], source_range[1], -1.0, 1.0)

def judge_to_consensus_scale(judge_score: float) -> float:
    """Convert judge [0,1] → consensus [-1,1]."""
    return normalize_to_range(judge_score, 0.0, 1.0, -1.0, 1.0)

def consensus_to_judge_scale(consensus_label: float) -> float:
    """Convert consensus [-1,1] → judge [0,1] for evaluation comparisons."""
    return normalize_to_range(consensus_label, -1.0, 1.0, 0.0, 1.0)


# ── Trajectory types ─────────────────────────────────────────────────────────
class TrajectoryType(str, Enum):
    HIGH_CONF_SUCCESS = "high_confidence_success"
    RECOVERY_SUCCESS  = "recovery_success"
    TESTED_FAILURE    = "tested_failure"
    UNTESTED_FAILURE  = "untested_failure"
    HIGH_CONF_FAILURE = "high_confidence_failure"
    UNKNOWN           = "unknown"


# ── Evaluation axes ──────────────────────────────────────────────────────────
VALID_AXES = frozenset(["correctness", "reasoning_coherence",
                         "action_appropriateness", "efficiency",
                         "task_understanding", "reasoning_quality", "process_quality"])  # Added merged axes

CODE_GENERATING_TOOLS    = frozenset(["submit_solution", "test_code_snippet"])
CODE_GENERATING_ACTIONS  = frozenset(["generate_code", "write_code", "implement"])


# ── Calibration thresholds ───────────────────────────────────────────────────
MIN_JUDGE_SPEARMAN_FOR_TRAINING = 0.65   # Below this: do not use judge labels
MIN_IAR_SPEARMAN                = 0.67   # Inter-annotator reliability floor
MIN_CALIBRATION_ANNOTATIONS     = 50     # Minimum for reliable calibration


# ── PRM training targets ─────────────────────────────────────────────────────
MIN_CONFIDENCE_FOR_TRAINING = 0.5        # Steps below this are excluded
MC_ZERO_DATA_CONFIDENCE     = 0.0        # mc_zero_data=True → always exclude
