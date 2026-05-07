#!/usr/bin/env python3
"""
Parallel MC PRM Labeler — v3

Fixes all 11 remaining issues found in v2.5 (document 2) review.
All prior fixes (v1 → v2 changelog) are preserved unchanged.

═══════════════════════════════════════════════════════════════════════
REMAINING BUGS FIXED IN v3
═══════════════════════════════════════════════════════════════════════

[BUG-A] CRITICAL — _seen_ids dedup broken under multiprocessing
  v2.5 attempted duplicate trajectory_id detection via a function
  attribute `load_and_build_meta._seen_ids`.  Each spawned worker
  process gets its own memory space, so `hasattr` returns False in
  every worker; _seen_ids is re-initialised fresh per worker.
  Cross-worker duplicates (the common case) are silently missed.
  Additionally the attribute persists across calls in the same process,
  corrupting subsequent runs in test suites.
  FIX: Dedup is performed in the main process after Pass 1 completes,
  on the sorted rollout_metas list.  Sorting first makes the "winner"
  of any collision deterministic (lexicographically first file path).

[BUG-B] HIGH — float round-trip precision loss in LOO base rate
  v2.5 ships task_base_rate (a float average) + task_traj_count to
  workers, which then reconstruct the numerator via multiplication:
      loo_base_numerator = task_base_rate * task_traj_count - self_contrib
  Dividing then multiplying back through float introduces error.
  Example: scores=[1,1,0], base_rate=0.6666..., 0.6666... * 3 = 1.9999...
  instead of 2.0.  For small beam counts this causes measurable bias.
  FIX: Ship the raw (numerator_sum, count) tuple in the new worker
  global _task_base_rate_stats_global.  Workers subtract self_contrib
  from the exact numerator — no multiply-back needed.
  label_trajectory_file args tuple shrinks from 5-tuple to 3-tuple.

[BUG-C] HIGH — wilson_ci called unconditionally in soft-MC mode
  v2.5 added a `if p_hat > 1.0: return 0.0, 1.0` guard but this only
  catches the degenerate case.  When 0 < p_hat <= 1 but loo_succ is a
  fractional sum (e.g., 1.5/3 = 0.5), the formula runs without error
  but produces a CI that has no statistical meaning — it's a binomial
  formula applied to non-integer counts.
  FIX: Gate wilson_ci behind `not _soft_mc_global`.  In soft-MC mode
  emit ci_low=0.0, ci_high=1.0, ci_valid=False as uninformative
  sentinels, consistent with the v3 full-file schema.

[BUG-D] HIGH — dual LOO position computation not fixed
  v2.5 still computes the identical subtraction twice per step:
    (1) in the smoothed_pos_scores pre-loop
    (2) inside the per-step for-loop
  Both loops execute `max(0, g_succ - self_contrib)` and
  `max(0, g_total - 1)` for every position k.
  FIX: Build loo_pos_cache: List[Tuple[float, int]] in a single pass
  before the step loop.  The step loop reads loo_pos_cache[pos].

[BUG-E] MEDIUM — mc_ci_valid field not emitted
  Whether CI bounds are statistically valid depends on the mode
  (hard-MC vs soft-MC).  Without this field in the output, consumers
  of both the full JSONL and the compact file have no way to filter
  or warn on invalid intervals short of re-reading the run config.
  FIX: Emit mc_ci_valid in both full-trajectory JSONL and compact JSONL.

[BUG-F] MEDIUM — MC stats computation is single-threaded in main process
  v2.5 iterates over all task_id groups serially:
      for task_id, rollouts in tqdm(task_groups.items(), ...):
          pos_s, pfx_s = compute_global_mc_stats(rollouts, ...)
  For 10 k tasks × 30 beams × 50 steps = 15 M accumulation ops in one
  thread.  This is a wall-clock bottleneck between Pass 1 and Pass 2.
  FIX: _compute_task_mc_stats is a module-level picklable wrapper
  (not a closure) so it can be dispatched via pool.imap_unordered.

[BUG-G] MEDIUM — --strict_mc flag referenced in changelog but absent
  The v2 changelog lists "[ML-H3] --strict_mc flag" as addressed, but
  the argparser has no such argument.  This is both misleading
  documentation and a missing safety net.
  FIX: --strict_mc added.  When set, main() aborts (sys.exit(1)) if
  avg_beams_per_task < 10, the threshold below which retroactive LOO
  approximation degrades significantly.

[BUG-H] MEDIUM — --compress_state unimplemented; _advance_state return discarded
  The v2 code comment says "To reduce file size set --compress_state",
  but --compress_state is not in the argparser and _advance_state's
  return value (the step delta) is always discarded in Pass 2.
  FIX: --compress_state flag added to argparser.
  _compress_state_global added to worker globals.
  In label_trajectory_file:
    - pending_delta accumulates ALL messages since the last LABELED
      step, covering skipped-step gaps, so full-state reconstruction
      is lossless for consumers.
    - First labeled step always stores full state (anchor).
    - Subsequent steps store state_delta only.
    - state_is_delta field emitted so consumers know the format.

[BUG-I] MEDIUM — unknown action type warning not aggregated in main()
  v2.5 added unknown_action_types to RolloutMeta (collected in
  load_and_build_meta) but never reads the field in main().
  The aggregation intended in [FIX-17] is absent.
  FIX: After Pass 1, collect all unique unknown types across all metas
  and emit a single warning if any are found.

[BUG-J] LOW — laplace_smooth type hint successes: int
  In soft-MC mode `successes` is a float sum of outcome_scores.
  The `int` annotation is incorrect and misleads static analysers.
  FIX: successes: float throughout.

[BUG-K] LOW — impossible bounds check on content_hash_seq
  `content_hash_seq[pos] if pos < len(content_hash_seq) else ""`
  content_hash_seq is built from raw_steps; pos iterates over
  enumerate(raw_steps); len(content_hash_seq) == len(raw_steps)
  always.  The guard can never trigger.
  FIX: Direct index `content_hash_seq[pos]` with an explanatory comment.

═══════════════════════════════════════════════════════════════════════
PERFORMANCE IMPROVEMENTS
═══════════════════════════════════════════════════════════════════════

[PERF-1] MC stats computation parallelized via pool.imap_unordered
  Uses the same worker pool as Pass 1 via _compute_task_mc_stats
  (module-level wrapper, pickle-safe).  No new pool overhead.

[PERF-2] LOO position data pre-computed once per trajectory  [BUG-D]
  Single loo_pos_cache list eliminates the duplicate subtraction pass.

[PERF-3] Compact JSONL writes batched per trajectory
  Instead of one f_compact.write() call per step, all steps for a
  trajectory are joined into a single string and written at once,
  reducing syscall count by a factor of ~avg_steps_per_traj.

═══════════════════════════════════════════════════════════════════════
ALL ISSUES (v1 → v2 → v2.5 → v3) NOW FIXED
═══════════════════════════════════════════════════════════════════════

Usage:
    python mc_prm_labeler_v3.py \\
        --results_dir       RESULTS_ECML_FINAL \\
        --output_dir        LABELED_RESULTS \\
        --workers           120 \\
        --min_rollouts      3 \\
        --success_threshold 1.0 \\
        --mc_hard_threshold 0.5 \\
        --laplace_alpha     1.0
"""

from __future__ import annotations

import hashlib
import json
import logging
import math
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from multiprocessing import Pool, cpu_count
from pathlib import Path
from types import MappingProxyType
from typing import Dict, List, Optional, Set, Tuple

import argparse
from tqdm import tqdm

logger = logging.getLogger("mc_prm_labeler")


# ─────────────────────────────────────────────────────────────────────────────
# Type aliases
#
# _StatTuple numerator is float throughout so that both hard-MC (integer 0/1
# accumulation) and soft-MC (fractional outcome_score sums) share one type.
# Previously annotated as Tuple[int, int] which was wrong for soft-MC.
# ─────────────────────────────────────────────────────────────────────────────

_StatTuple  = Tuple[float, int]        # (numerator_sum, total_count)
_PosStats   = Dict[int, _StatTuple]    # step position       → stat
_PfxStats   = Dict[str, _StatTuple]    # type-seq fingerprint → stat
_BaseStats  = Tuple[float, int]        # (numerator_sum, total_count) for base rate


# ─────────────────────────────────────────────────────────────────────────────
# Worker-process globals — initialised at module level  [ENG-L1]
# Set once per worker via _init_worker; never re-serialised per task.
# ─────────────────────────────────────────────────────────────────────────────

_global_pos_stats: Dict[str, _PosStats] = {}
_global_pfx_stats: Dict[str, _PfxStats] = {}

# 3-tuple: (task_id, is_success, outcome_score).
# action_type_seq intentionally excluded — recomputed in worker from raw JSON;
# shipping it would pickle O(workers × n_trajs × seq_len) strings uselessly.
_traj_meta_global: Dict[str, Tuple[str, bool, float]] = {}

# [BUG-B FIX] Raw (numerator_sum, count) per task so workers can compute the
# LOO-adjusted base rate via exact subtraction — no multiply-back float error.
_task_base_rate_stats_global: Dict[str, _BaseStats] = {}

_min_rollouts_global:            int   = 1
_exclude_ff_global:              bool  = False
_mc_hard_threshold_global:       float = 0.5
_laplace_alpha_global:           float = 1.0
_success_threshold_global:       float = 1.0
_soft_mc_global:                 bool  = False
_advantage_neutral_band_global:  float = 0.05
_compress_state_global:          bool  = False   # [BUG-H FIX]
_length_normalize_global:        bool  = True    # Length normalization for trajectory bias
_reward_shaping_global:          bool  = True    # Step-type reward shaping


def _init_worker(
    global_pos_stats:          Dict,
    global_pfx_stats:          Dict,
    traj_meta:                 Dict,
    task_base_rate_stats:      Dict,   # [BUG-B FIX] raw (num, count) per task
    min_rollouts:              int,
    exclude_ff:                bool,
    mc_hard_threshold:         float,
    laplace_alpha:             float,
    success_threshold:         float,
    soft_mc:                   bool,
    advantage_neutral_band:    float,
    compress_state:            bool,   # [BUG-H FIX]
    length_normalize:          bool,
    reward_shaping:            bool,
) -> None:
    """Called once per worker at Pool startup."""
    global _global_pos_stats, _global_pfx_stats, _traj_meta_global
    global _task_base_rate_stats_global
    global _min_rollouts_global, _exclude_ff_global
    global _mc_hard_threshold_global, _laplace_alpha_global
    global _success_threshold_global, _soft_mc_global
    global _advantage_neutral_band_global, _compress_state_global
    global _length_normalize_global, _reward_shaping_global

    _global_pos_stats              = global_pos_stats
    _global_pfx_stats              = global_pfx_stats
    _traj_meta_global              = traj_meta
    _task_base_rate_stats_global   = task_base_rate_stats
    _min_rollouts_global           = min_rollouts
    _exclude_ff_global             = exclude_ff
    _mc_hard_threshold_global      = mc_hard_threshold
    _laplace_alpha_global          = laplace_alpha
    _success_threshold_global      = success_threshold
    _soft_mc_global                = soft_mc
    _advantage_neutral_band_global = advantage_neutral_band
    _compress_state_global         = compress_state
    _length_normalize_global       = length_normalize
    _reward_shaping_global         = reward_shaping


# ─────────────────────────────────────────────────────────────────────────────
# Constants  [ENG-M2]
# ─────────────────────────────────────────────────────────────────────────────

_ABBREV: MappingProxyType = MappingProxyType({
    "reasoning":          "R",
    "generate_code":      "GC",
    "tool_call":          "TC",
    "forced_final_answer": "FF",   # Fix for prefix fingerprint corruption
})

SKIP_TYPES = frozenset({"forced_final_answer"})


# ─────────────────────────────────────────────────────────────────────────────
# Data structures
# ─────────────────────────────────────────────────────────────────────────────

@dataclass
class RolloutMeta:
    trajectory_id:        str
    task_id:              str
    provider:             str
    model:                str
    n_steps:              int
    success:              bool
    outcome_score:        float
    # Kept in RolloutMeta for compute_global_mc_stats (main-process only).
    # NOT forwarded into _traj_meta_global shipped to labeling workers.
    action_type_seq:      List[str] = field(default_factory=list)
    file_path:            str       = ""
    relative_path:        str       = ""
    # [BUG-I FIX] Collected here for a single aggregated warning in main().
    unknown_action_types: List[str] = field(default_factory=list)


# ─────────────────────────────────────────────────────────────────────────────
# Incremental diagnostics — zero per-step memory  [NEW-BUG-3 FIX from v2]
# Welford online algorithm; no stored per-step values needed.
# ─────────────────────────────────────────────────────────────────────────────

@dataclass
class IncrementalStats:
    """Welford online Pearson correlation tracker."""
    n:              int   = 0
    # Position vs mc_label_soft  [ML-M8]
    sum_pos:        float = 0.0
    sum_score:      float = 0.0
    sum_pos2:       float = 0.0
    sum_score2:     float = 0.0
    sum_cross:      float = 0.0
    # mc_label_prefix_soft vs mc_label_soft  [ML-L1]
    sum_pfx:        float = 0.0
    sum_pfx2:       float = 0.0
    sum_pfx_x_soft: float = 0.0
    # Advantage sign counts  [ML-H4]
    adv_pos:        int   = 0
    adv_neg:        int   = 0
    adv_zero:       int   = 0

    def update(
        self,
        step_position: int,
        mc_soft:       float,
        pfx_soft:      float,
        advantage:     float,
    ) -> None:
        self.n += 1
        pos_f = float(step_position)
        self.sum_pos        += pos_f
        self.sum_score      += mc_soft
        self.sum_pfx        += pfx_soft
        self.sum_pos2       += pos_f    * pos_f
        self.sum_score2     += mc_soft  * mc_soft
        self.sum_pfx2       += pfx_soft * pfx_soft
        self.sum_cross      += pos_f    * mc_soft
        self.sum_pfx_x_soft += pfx_soft * mc_soft
        if advantage > 0.01:
            self.adv_pos  += 1
        elif advantage < -0.01:
            self.adv_neg  += 1
        else:
            self.adv_zero += 1

    def _pearson(
        self,
        sx: float, sy: float,
        sx2: float, sy2: float,
        sxy: float,
    ) -> Optional[float]:
        if self.n < 2:
            return None
        n     = self.n
        denom = math.sqrt(
            max(0.0, n * sx2 - sx * sx) *
            max(0.0, n * sy2 - sy * sy)
        )
        return None if denom == 0 else (n * sxy - sx * sy) / denom

    def position_score_corr(self) -> Optional[float]:
        return self._pearson(
            self.sum_pos,   self.sum_score,
            self.sum_pos2,  self.sum_score2,
            self.sum_cross,
        )

    def prefix_soft_corr(self) -> Optional[float]:
        return self._pearson(
            self.sum_score, self.sum_pfx,
            self.sum_score2, self.sum_pfx2,
            self.sum_pfx_x_soft,
        )


# ─────────────────────────────────────────────────────────────────────────────
# Success / outcome extraction  [ENG-H3] [ML-CRITICAL-2]
# ─────────────────────────────────────────────────────────────────────────────

def extract_outcome(traj: Dict, threshold: float = 1.0) -> Tuple[bool, float]:
    """
    Returns (is_success, continuous_score).  success = score >= threshold.

    NOTE [ML-H3]: Retroactive labeling using existing rollouts only.
    True MC-PRM requires sampling new completions from each step state.
    This approximation degrades below ~10 beams per task.
    
    [FIXED-v4] Corrected to handle actual JSON structure:
    - final_submission is a dict with 'compiled' and 'test_results' fields
    - test_results can be nested inside final_submission
    - observation.compiled is a direct success indicator
    """
    task_id = traj.get("task_id", "?")

    # ─────────────────────────────────────────────────────────────────────
    # Check 1: passed_tests at root (most direct indicator)
    # ─────────────────────────────────────────────────────────────────────
    if "passed_tests" in traj:
        score = 1.0 if traj["passed_tests"] else 0.0
        return score >= threshold, score

    # ─────────────────────────────────────────────────────────────────────
    # Check 2: Root-level test_results.returncode
    # ─────────────────────────────────────────────────────────────────────
    if "test_results" in traj and isinstance(traj["test_results"], dict):
        tr = traj["test_results"]
        if "returncode" in tr:
            score = 1.0 if tr["returncode"] == 0 else 0.0
            return score >= threshold, score
        if "stdout" in tr and isinstance(tr["stdout"], str):
            if "100% tests passed" in tr["stdout"]:
                return True, 1.0

    # ─────────────────────────────────────────────────────────────────────
    # Check 3: final_submission — test_results.returncode first, then compiled
    # NOTE: compiled=True only means it compiled, NOT that tests passed.
    # Prefer test_results.returncode when available.
    # ─────────────────────────────────────────────────────────────────────
    if "final_submission" in traj and isinstance(traj["final_submission"], dict):
        fs = traj["final_submission"]
        if "test_results" in fs and isinstance(fs["test_results"], dict):
            tr = fs["test_results"]
            if "returncode" in tr:
                score = 1.0 if tr["returncode"] == 0 else 0.0
                return score >= threshold, score
        # compiled alone is a last resort — does not confirm tests passed
        if "compiled" in fs and fs["compiled"] is False:
            return False, 0.0

    # ─────────────────────────────────────────────────────────────────────
    # Check 4: Trajectory steps — llm_view (dict only, not list)
    # ─────────────────────────────────────────────────────────────────────
    for step in reversed(traj.get("trajectory", [])):
        obs = step.get("observation")
        if not isinstance(obs, dict):
            continue
        if "llm_view" in obs and isinstance(obs["llm_view"], dict):
            lv = obs["llm_view"]
            if "compiles" in lv and "tests_pass" in lv:
                score = 1.0 if (lv["compiles"] and lv["tests_pass"]) else 0.0
                return score >= threshold, score
        if "full_result" in obs and isinstance(obs["full_result"], dict):
            fr = obs["full_result"]
            if "returncode" in fr:
                score = 1.0 if fr["returncode"] == 0 else 0.0
                return score >= threshold, score

    return False, 0.0


# ─────────────────────────────────────────────────────────────────────────────
# Fingerprinting  [ML-H2] [ENG-M1] [NEW-BUG-2 FIX]
# ─────────────────────────────────────────────────────────────────────────────

def action_type_prefix_fp(action_type_seq: List[str], up_to: int) -> str:
    """
    Type-sequence-only prefix fingerprint (up_to is exclusive upper bound).
    Call with up_to=k+1 to include step k.  [ENG-M1]

    [BUG-I] Types absent from _ABBREV map to 'X' (same as missing-type steps).
    Aggregated warning emitted in main() via RolloutMeta.unknown_action_types.
    """
    return "|".join(_ABBREV.get(t, "X") for t in action_type_seq[:up_to])


def generate_trajectory_id_hash(
    provider: str,
    model: str, 
    task_id: str,
    beam_id: str,
    file_path: Path,
    results_dir: Path
) -> str:
    """
    Generate unique trajectory ID using pure hash-based approach.
    
    Creates a deterministic hash from all identifying components plus file path
    to guarantee no collisions even across different batch folders or runs.
    """
    rel_path = str(file_path.relative_to(results_dir))
    
    # Create unique key from all components
    unique_key = f"{provider}|{model}|{task_id}|{beam_id}|{rel_path}"
    
    # Generate deterministic hash
    trajectory_hash = hashlib.md5(unique_key.encode('utf-8')).hexdigest()
    
    # Return hash-based ID with human-readable prefix
    return f"{provider}__{model}__{task_id}__beam_{beam_id}__hash_{trajectory_hash[:12]}"
def content_hash_of_action(action: Dict) -> str:
    """MD5 of action content — metadata only; NOT used as a grouping key."""
    content = (
        action.get("content") or
        action.get("code")    or
        action.get("reasoning") or
        json.dumps(action, sort_keys=True, default=str)
    )
    return hashlib.md5(
        str(content).encode("utf-8", errors="replace")
    ).hexdigest()[:10]


# ─────────────────────────────────────────────────────────────────────────────
# Statistical helpers  [ML-M2] [ML-L2]
# ─────────────────────────────────────────────────────────────────────────────

def laplace_smooth(
    successes: float,   # [BUG-J FIX] float, not int — compatible with soft-MC
    trials:    int,
    alpha:     float = 1.0,
) -> float:
    """Add-alpha smoothed proportion.  With alpha>0: never exact 0.0 or 1.0."""
    return (successes + alpha) / (trials + 2.0 * alpha)


def wilson_ci(
    successes: float,
    trials:    int,
    z:         float = 1.96,
) -> Tuple[float, float]:
    """
    Wilson score 95% CI for a Bernoulli proportion.

    [BUG-C FIX] ONLY valid when successes is an integer count (hard-MC).
    In soft-MC mode loo_succ is a fractional sum of outcome_scores; this
    function must NOT be called then.  label_trajectory_file gates on
    _soft_mc_global and emits mc_ci_valid=False with sentinel (0.0, 1.0).
    """
    if trials == 0:
        return 0.0, 1.0
    p_hat  = successes / trials
    denom  = 1.0 + z * z / trials
    center = (p_hat + z * z / (2.0 * trials)) / denom
    margin = (z * math.sqrt(
        p_hat * (1.0 - p_hat) / trials + z * z / (4.0 * trials * trials)
    )) / denom
    return max(0.0, center - margin), min(1.0, center + margin)


# ─────────────────────────────────────────────────────────────────────────────
# Pass 1 worker  [ENG-L2] [ML-L5] [ML-CRITICAL-3]
# ─────────────────────────────────────────────────────────────────────────────

def load_and_build_meta(
    args: Tuple[Path, Path, float],
) -> Optional[RolloutMeta]:
    """
    Load one trajectory file and return its RolloutMeta.

    [BUG-A FIX] Duplicate detection was removed from this function.
    It used a function attribute (_seen_ids) that is local to each worker
    process, so cross-worker duplicates were never detected.  Dedup now
    happens in the main process on the sorted result list.
    """
    file_path, results_dir, success_threshold = args
    try:
        with open(file_path) as f:
            traj = json.load(f)

        task_id = traj.get("task_id", "")
        if not task_id:
            logger.warning(
                "Empty or missing task_id in %s — skipping to prevent "
                "cross-task MC stat contamination.",
                file_path,
            )
            return None

        raw_beam_id = traj.get("beam_id")
        if raw_beam_id is None:
            # Always derive beam_id from the file path
            # Extract from filename pattern: *_beam_N_log_trj_repaired.json
            filename = file_path.name
            import re
            beam_match = re.search(r'_beam_(\d+)_log_trj_repaired\.json$', filename)
            if beam_match:
                raw_beam_id = int(beam_match.group(1))
            else:
                # Fallback to hash if pattern doesn't match
                raw_beam_id = (
                    "fp_" + hashlib.md5(str(file_path).encode()).hexdigest()[:8]
                )
        beam_id = str(raw_beam_id)

        rel      = file_path.relative_to(results_dir)
        parts    = rel.parts
        provider = parts[0] if len(parts) > 0 else "unknown"
        model    = parts[1] if len(parts) > 1 else "unknown"
        
        # Use hash-based trajectory ID generation for guaranteed uniqueness
        trajectory_id = generate_trajectory_id_hash(
            provider=provider,
            model=model,
            task_id=task_id,
            beam_id=beam_id,
            file_path=file_path,
            results_dir=results_dir
        )

        steps = traj.get("trajectory", [])
        action_type_seq = [
            (s.get("action") or {}).get("type", "X") for s in steps
        ]

        # [BUG-I FIX] Collect unknown types for aggregated main() warning.
        unknown_types = sorted({
            t for t in action_type_seq
            if t not in _ABBREV and t != "X"
        })

        is_success, outcome_score = extract_outcome(traj, threshold=success_threshold)

        return RolloutMeta(
            trajectory_id        = trajectory_id,
            task_id              = task_id,
            provider             = provider,
            model                = model,
            n_steps              = len(steps),
            success              = is_success,
            outcome_score        = outcome_score,
            action_type_seq      = action_type_seq,
            file_path            = str(file_path),
            relative_path        = str(rel),
            unknown_action_types = unknown_types,
        )

    except Exception as exc:
        logger.warning(
            "Failed to load %s — %s: %s",
            file_path, type(exc).__name__, exc,
        )
        return None


# ─────────────────────────────────────────────────────────────────────────────
# Global MC stats (per task_id, cross-model pooled)
# ─────────────────────────────────────────────────────────────────────────────

def compute_global_mc_stats(
    rollouts:         List[RolloutMeta],
    use_soft_outcome: bool = False,
) -> Tuple[_PosStats, _PfxStats]:
    """
    Non-LOO stats for one task_id group.
    Numerators are float to support both hard-MC and soft-MC modes.
    """
    max_steps = max((r.n_steps for r in rollouts), default=0)
    pos: Dict[int, List[float]] = {k: [0.0, 0] for k in range(max_steps)}
    pfx: Dict[str, List[float]] = defaultdict(lambda: [0.0, 0])

    for r in rollouts:
        c = r.outcome_score if use_soft_outcome else (1.0 if r.success else 0.0)
        for k in range(r.n_steps):
            pos[k][1] += 1
            pos[k][0] += c
            fp = action_type_prefix_fp(r.action_type_seq, up_to=k + 1)
            pfx[fp][1] += 1
            pfx[fp][0] += c

    return (
        {k: (v[0], int(v[1])) for k, v in pos.items()},
        {k: (v[0], int(v[1])) for k, v in pfx.items()},
    )


def _compute_task_mc_stats(
    args: Tuple[str, List[RolloutMeta], bool],
) -> Tuple[str, _PosStats, _PfxStats]:
    """
    Module-level picklable wrapper for compute_global_mc_stats.
    [PERF-1 / BUG-F FIX] Must be at module level (not a closure) to be
    dispatched via multiprocessing.Pool.imap_unordered.
    """
    task_id, rollouts, use_soft_outcome = args
    pos_s, pfx_s = compute_global_mc_stats(rollouts, use_soft_outcome)
    return task_id, pos_s, pfx_s


# ─────────────────────────────────────────────────────────────────────────────
# Per-step advantage  [ML-H4]
# ─────────────────────────────────────────────────────────────────────────────

def compute_advantages(
    mc_scores:     Dict[int, float],
    base_rate:     float,
    all_positions: int,
) -> Dict[int, float]:
    """
    A(k) = Q(k) - Q(k-1),  Q(-1) = LOO-adjusted task base success rate.

    [BUG-B FIX] base_rate is now the LOO-adjusted value, computed in the
    worker from the exact raw (numerator, count) stats — no float round-trip.

    [ML-NEW-1] Gap-aware: walks back to nearest preceding position with data.
    Positions with no MC data get advantage=0.0 (neutral, never blamed).
    """
    advantages: Dict[int, float] = {}
    for k in range(all_positions):
        if k not in mc_scores:
            advantages[k] = 0.0
            continue
        prev_k = k - 1
        while prev_k >= 0 and prev_k not in mc_scores:
            prev_k -= 1
        prev_q = mc_scores[prev_k] if prev_k >= 0 else base_rate
        advantages[k] = mc_scores[k] - prev_q
    return advantages


# ─────────────────────────────────────────────────────────────────────────────
# Error type detection  [NEW-FIX: Compile vs Logic vs Tool errors]
# ─────────────────────────────────────────────────────────────────────────────

def extract_error_type(step: Dict) -> str:
    """
    Determine the actual error type from step observation.
    
    CRITICAL FIX: Prioritize full_result over llm_view since llm_view can be incorrect.
    Example: Step 12 shows llm_view.compiles=true but full_result.returncode=2 (compile failed).
    
    Returns: "success", "compile_error", "logic_error", "tool_error", or "unknown"
    """
    obs = step.get("observation")
    if obs is None:
        return "unknown"
    
    # PRIORITY 1: Check full_result (ground truth)
    full_result = obs.get("full_result", {})
    if isinstance(full_result, dict):
        # Tool execution errors
        if "error" in full_result:
            error_msg = str(full_result.get("error", ""))
            if "Tool execution failed" in error_msg or "AgenticTools" in error_msg:
                return "tool_error"
        
        # Compilation/test results from returncode
        returncode = full_result.get("returncode")
        if returncode is not None:
            if returncode == 0:
                return "success"
            elif returncode == 2:  # Compilation error
                return "compile_error"
            else:  # returncode == 1 or other non-zero
                return "logic_error"
    
    # PRIORITY 2: Check llm_view (can be incorrect, use as fallback)
    llm_view = obs.get("llm_view", obs) if isinstance(obs, dict) else obs
    if isinstance(llm_view, dict):
        compiles = llm_view.get("compiles")
        tests_pass = llm_view.get("tests_pass")
        
        if compiles is False:
            return "compile_error"
        if compiles is True and tests_pass is False:
            return "logic_error"
        if compiles is True and tests_pass is True:
            return "success"
    
    # PRIORITY 3: Check observation.compiled directly
    if isinstance(obs, dict):
        compiled = obs.get("compiled")
        tests_pass = obs.get("tests_pass")
        
        if compiled is False:
            return "compile_error"
        if compiled is True and tests_pass is False:
            return "logic_error"
        if compiled is True and tests_pass is True:
            return "success"
    
    return "unknown"


def extract_step_reward(step: Dict, error_type: str, enable_shaping: bool = True) -> float:
    """
    Assign step-level reward based on error type and observation.
    
    Reward scheme (when enable_shaping=True):
    - Test pass: +1.0
    - Compile success: +0.2
    - Code generation: +0.05
    - Tool success: +0.05
    - Tool error: -0.1
    - Compile error: -0.2
    - Logic error (test fail): -1.0
    
    When enable_shaping=False, only returns final outcome (1.0 or -1.0).
    """
    if not enable_shaping:
        # Simple binary reward
        return 1.0 if error_type == "success" else -1.0
    
    reward = 0.0
    action = step.get("action", {})
    action_type = action.get("type", "")
    obs = step.get("observation", {})
    
    # Code generation step
    if action_type in ("generate_code", "forced_final_answer"):
        reward += 0.05
    
    # Check observation for compile/test results
    if isinstance(obs, dict):
        llm_view = obs.get("llm_view", obs)
        if isinstance(llm_view, dict):
            compiles = llm_view.get("compiles")
            tests_pass = llm_view.get("tests_pass")
            
            if compiles is True:
                reward += 0.2
            elif compiles is False:
                reward -= 0.2
            
            if tests_pass is True:
                reward += 1.0
            elif tests_pass is False:
                reward -= 1.0
        
        # Alternative observation format
        compiled = obs.get("compiled")
        tests = obs.get("tests_pass")
        if compiled is True:
            reward += 0.2
        elif compiled is False:
            reward -= 0.2
        if tests is True:
            reward += 1.0
        elif tests is False:
            reward -= 1.0
    
    # Tool call rewards
    if action_type == "tool_call":
        if isinstance(obs, dict):
            full_result = obs.get("full_result", {})
            if isinstance(full_result, dict):
                if "error" not in full_result:
                    reward += 0.05
                else:
                    reward -= 0.1
    
    # Fallback to error_type if no observation signals
    if reward == 0.0:
        if error_type == "success":
            reward = 1.0
        elif error_type == "compile_error":
            reward = -0.2
        elif error_type == "logic_error":
            reward = -1.0
        elif error_type == "tool_error":
            reward = -0.1
    
    return reward


def normalize_reasoning(step: Dict) -> str:
    """
    Extract reasoning from multiple possible fields.
    
    Reasoning can appear in:
    - step["reasoning"]
    - step["reasoning_content"]
    - step["content"]
    - action["content"]
    - action["reasoning"]
    
    If multiple fields exist and are >80% different, merge them.
    Otherwise return the first non-empty value found.
    """
    candidates = []
    
    # Collect all non-empty reasoning fields
    reasoning = step.get("reasoning")
    if reasoning:
        candidates.append(("reasoning", str(reasoning)))
    
    reasoning_content = step.get("reasoning_content")
    if reasoning_content:
        candidates.append(("reasoning_content", str(reasoning_content)))
    
    content = step.get("content")
    if content:
        candidates.append(("content", str(content)))
    
    # Check action-level fields
    action = step.get("action", {})
    if isinstance(action, dict):
        action_content = action.get("content")
        if action_content:
            candidates.append(("action.content", str(action_content)))
        
        action_reasoning = action.get("reasoning")
        if action_reasoning:
            candidates.append(("action.reasoning", str(action_reasoning)))
    
    if not candidates:
        return ""
    
    # If only one candidate, return it
    if len(candidates) == 1:
        return candidates[0][1]
    
    # Check if candidates are significantly different (>80% different)
    # Use simple character-level difference ratio
    def similarity_ratio(s1: str, s2: str) -> float:
        """Calculate similarity ratio between two strings (0.0 = completely different, 1.0 = identical)"""
        if not s1 and not s2:
            return 1.0
        if not s1 or not s2:
            return 0.0
        
        # Simple character overlap ratio
        set1 = set(s1.lower())
        set2 = set(s2.lower())
        if not set1 and not set2:
            return 1.0
        if not set1 or not set2:
            return 0.0
        
        intersection = len(set1 & set2)
        union = len(set1 | set2)
        return intersection / union if union > 0 else 0.0
    
    # Check if any two candidates are >80% different (similarity < 0.2)
    should_merge = False
    for i in range(len(candidates)):
        for j in range(i + 1, len(candidates)):
            sim = similarity_ratio(candidates[i][1], candidates[j][1])
            if sim < 0.2:  # >80% different
                should_merge = True
                break
        if should_merge:
            break
    
    if should_merge:
        # Merge all unique candidates with field labels
        merged_parts = []
        seen_content = set()
        for field_name, text in candidates:
            # Avoid duplicates
            text_lower = text.lower().strip()
            if text_lower and text_lower not in seen_content:
                merged_parts.append(f"[{field_name}] {text}")
                seen_content.add(text_lower)
        return "\n\n".join(merged_parts)
    else:
        # All candidates are similar, return the first (longest) one
        return max(candidates, key=lambda x: len(x[1]))[1]


# ─────────────────────────────────────────────────────────────────────────────
# State management helpers  [ENG-L3] [BUG-H FIX]
# ─────────────────────────────────────────────────────────────────────────────

def _advance_state(
    cumulative: List[Dict],
    action:     Dict,
    observation,
) -> List[Dict]:
    """
    Append this step's messages to `cumulative`.  Return the delta list.

    [BUG-H FIX] Return value is now consumed by callers in compress_state
    mode: they extend pending_delta with the returned messages so that
    every message since the last labeled step is captured even across
    skipped-step gaps (min_rollouts / filtered-failure drops).
    """
    delta: List[Dict] = []
    content = (
        action.get("content") or
        action.get("code")    or
        action.get("reasoning") or ""
    )
    if content:
        msg = {"role": "assistant", "content": str(content)}
        delta.append(msg)
        cumulative.append(msg)

    if observation is not None:
        obs_text = (
            observation if isinstance(observation, str)
            else json.dumps(observation, default=str)
        )
        msg = {"role": "tool", "content": obs_text}
        delta.append(msg)
        cumulative.append(msg)

    return delta


# ─────────────────────────────────────────────────────────────────────────────
# Pass 2 worker — LOO computed on-the-fly  [NEW-BUG-1 FIX]
#
# For step k in trajectory T:
#   loo_succ  = global_succ  - self_contrib
#   loo_total = global_total - 1
# O(1) per step, O(1) extra memory per trajectory.
# ─────────────────────────────────────────────────────────────────────────────

def label_trajectory_file(
    args: Tuple[str, str, str],
) -> Tuple[str, List[Dict]]:
    """
    Returns (relative_path, labeled_steps).
    args: (file_path, relative_path, trajectory_id)

    [BUG-B FIX] task_base_rate and task_traj_count removed from args tuple.
    Workers derive the LOO base rate directly from _task_base_rate_stats_global
    using exact (numerator_sum, count) — no multiply-back float error.
    """
    file_path, relative_path, trajectory_id = args

    try:
        with open(file_path) as f:
            traj = json.load(f)
    except (json.JSONDecodeError, OSError) as exc:
        logger.error("Cannot read %s: %s", file_path, exc)
        return (relative_path, [])

    task_id     = traj.get("task_id", "")
    repo        = traj.get("repo", "")
    traj_model  = traj.get("model", "")
    temperature = traj.get("temperature", 0.0)
    raw_beam_id_for_storage = traj.get("beam_id", 0)

    # [ML-M7] Abort early if no stats were built for this task
    task_pos_stats = _global_pos_stats.get(task_id)
    task_pfx_stats = _global_pfx_stats.get(task_id)
    if task_pos_stats is None or task_pfx_stats is None:
        logger.error(
            "No global MC stats for task_id=%r (trajectory_id=%r). "
            "Pass 1 likely failed to load any trajectory for this task.",
            task_id, trajectory_id,
        )
        return (relative_path, [])

    traj_meta_entry = _traj_meta_global.get(trajectory_id)
    if traj_meta_entry is None:
        logger.error("No traj_meta for trajectory_id=%r — skipping.", trajectory_id)
        return (relative_path, [])

    _meta_task_id, self_success, self_outcome_score = traj_meta_entry

    # [ML-NEW-4] In soft-MC mode self_contrib is the continuous outcome_score
    self_contrib: float = (
        self_outcome_score if _soft_mc_global
        else (1.0 if self_success else 0.0)
    )

    raw_steps        = traj.get("trajectory", [])
    action_type_seq  = [(s.get("action") or {}).get("type", "X") for s in raw_steps]
    content_hash_seq = [content_hash_of_action(s.get("action") or {}) for s in raw_steps]

    outcome_bool_pass2, outcome_score = extract_outcome(
        traj, threshold=_success_threshold_global
    )
    outcome = 1 if outcome_bool_pass2 else 0

    # [BUG-1 FIX] Define n_steps BEFORE using it for length normalization
    n_steps = len(raw_steps)
    
    # Length normalization for MC return
    length_norm_factor = 1.0
    if _length_normalize_global and n_steps > 0:
        length_norm_factor = 1.0 / math.sqrt(n_steps)

    # [ML-NEW-2] Guard: Pass1 and Pass2 outcomes must agree
    if bool(self_success) != outcome_bool_pass2:
        logger.error(
            "Outcome inconsistency for trajectory_id=%r: "
            "Pass1 success=%s but Pass2 outcome=%s. "
            "MC labels may be slightly off; LOO uses Pass1 value.",
            trajectory_id, self_success, outcome_bool_pass2,
        )

    # [BUG-B FIX] Derive LOO base rate from raw (numerator, count) — no
    # multiply-back.  This is exact regardless of floating-point representation.
    base_num, base_count = _task_base_rate_stats_global.get(task_id, (0.0, 0))
    loo_base_num   = max(0.0, base_num   - self_contrib)
    loo_base_count = max(0,   base_count - 1)
    loo_task_base_rate = (
        loo_base_num / loo_base_count if loo_base_count > 0 else 0.5
    )

    # [BUG-D FIX] Pre-compute ALL LOO position data once before the step loop.
    # v2.5 recomputed the identical subtraction twice: once here and once in
    # the per-step loop.  loo_pos_cache[k] = (loo_succ, loo_total).
    loo_pos_cache: List[Tuple[float, int]] = []
    smoothed_pos_scores: Dict[int, float] = {}

    for k in range(n_steps):
        g_succ, g_total = task_pos_stats.get(k, (0.0, 0))
        ls = max(0.0, float(g_succ)  - self_contrib)
        lt = max(0,   int(g_total)   - 1)
        loo_pos_cache.append((ls, lt))
        if lt > 0:
            smoothed_pos_scores[k] = laplace_smooth(ls, lt, _laplace_alpha_global)

    # [BUG-B FIX] Advantage prior uses the LOO-adjusted base rate
    advantages = compute_advantages(smoothed_pos_scores, loo_task_base_rate, n_steps)

    # Build initial conversation state
    initial_state: List[Dict] = []
    if traj.get("system_prompt"):
        initial_state.append({"role": "system", "content": traj["system_prompt"]})
    if traj.get("user_prompt"):
        initial_state.append({"role": "user",   "content": traj["user_prompt"]})
    cumulative_messages = list(initial_state)

    labeled_steps: List[Dict] = []
    sequence_idx  = 0

    # [BUG-H FIX] Compress-state tracking.
    # pending_delta: all messages since the last LABELED step, including
    # messages from steps skipped by min_rollouts or filtered-failure.
    # This ensures full-state reconstruction is lossless even across gaps.
    # first_labeled: first labeled step always stores full state (anchor).
    first_labeled  = True
    pending_delta: List[Dict] = []

    for pos, step in enumerate(raw_steps):
        action      = step.get("action") or {}
        action_type = action.get("type", "X")
        is_forced_final = action_type in SKIP_TYPES  # flag; never drop [ML-M4]

        # [CRITICAL FIX] For forced_final_answer and last code generation step,
        # pull observation from traj["final_submission"] and traj["test_results"]
        # instead of step.get("observation") which is incomplete.
        if action_type == "forced_final_answer" or (action_type == "generate_code" and pos == len(raw_steps) - 1):
            # Build complete observation from final_submission and test_results
            final_obs = {}
            
            # Extract from final_submission
            if "final_submission" in traj:
                fs = traj["final_submission"]
                if isinstance(fs, dict):
                    final_obs["compiled"] = fs.get("compiled", True)
                    final_obs["compile_error"] = fs.get("compile_error")
                    
                    # Check for test_results nested in final_submission
                    if "test_results" in fs and isinstance(fs["test_results"], dict):
                        tr = fs["test_results"]
                        final_obs["tests_pass"] = tr.get("returncode") == 0
                        final_obs["returncode"] = tr.get("returncode")
                        final_obs["stdout"] = tr.get("stdout")
                        final_obs["stderr"] = tr.get("stderr")
                        final_obs["errors"] = tr.get("errors", [])
            
            # Extract from root-level test_results (if not already in final_submission)
            if "test_results" in traj and "returncode" not in final_obs:
                tr = traj["test_results"]
                if isinstance(tr, dict):
                    final_obs["tests_pass"] = tr.get("returncode") == 0
                    final_obs["returncode"] = tr.get("returncode")
                    final_obs["stdout"] = tr.get("stdout")
                    final_obs["stderr"] = tr.get("stderr")
                    final_obs["errors"] = tr.get("errors", [])
            
            # Merge with existing observation (final_obs takes priority)
            step_obs = step.get("observation", {})
            if isinstance(step_obs, dict):
                step_obs.update(final_obs)
            else:
                step_obs = final_obs
            
            # Create a modified step with complete observation
            step = dict(step)
            step["observation"] = step_obs

        # [NEW-FIX] Extract correct error type (fixes compile/logic/tool error confusion)
        error_type = extract_error_type(step)
        step_reward = extract_step_reward(step, error_type, enable_shaping=_reward_shaping_global)

        # [BUG-D FIX] Read pre-computed cache — no recomputation
        loo_succ, loo_total = loo_pos_cache[pos]

        # Skip: insufficient LOO rollouts
        if loo_total < _min_rollouts_global and not is_forced_final:
            delta = _advance_state(cumulative_messages, action, step.get("observation"))
            if _compress_state_global:
                pending_delta.extend(delta)   # [BUG-H FIX] carry gap messages
            continue

        mc_soft = laplace_smooth(loo_succ, loo_total, _laplace_alpha_global)
        
        # [IMPROVEMENT-2 FIX] Integrate reward shaping into MC returns
        # When enabled, blend step-level reward signals into the MC estimate
        if _reward_shaping_global and step_reward != 0.0:
            # Blend: 90% MC estimate + 10% step reward signal
            # This provides dense learning signals while preserving MC calibration
            mc_soft = 0.9 * mc_soft + 0.1 * (0.5 + 0.5 * step_reward)
        
        mc_hard = 1 if mc_soft >= _mc_hard_threshold_global else 0

        # [BUG-C FIX] Wilson CI requires integer binomial counts.
        # In soft-MC mode loo_succ is a fractional sum; calling the CI formula
        # would silently produce a meaningless result.  Emit sentinels instead.
        if _soft_mc_global:
            ci_low, ci_high = 0.0, 1.0
            ci_valid = False
        else:
            ci_low, ci_high = wilson_ci(loo_succ, loo_total)
            ci_valid = True

        mc_reliable  = loo_total >= 10
        mc_zero_data = loo_total == 0   # [ML-NEW-3] pure Laplace prior

        fp = action_type_prefix_fp(action_type_seq, up_to=pos + 1)
        pf_g_succ, pf_g_total = task_pfx_stats.get(fp, (0.0, 0))
        pf_loo_succ  = max(0.0, float(pf_g_succ)  - self_contrib)
        pf_loo_total = max(0,   int(pf_g_total)    - 1)
        pfx_soft           = laplace_smooth(pf_loo_succ, pf_loo_total, _laplace_alpha_global)
        pfx_hard           = 1 if pfx_soft >= _mc_hard_threshold_global else 0
        mc_prefix_reliable = pf_loo_total >= 10   # [ML-NEW-M2]

        advantage = advantages.get(pos, 0.0)
        band      = _advantage_neutral_band_global
        adv_sign  = (1 if advantage > band else (-1 if advantage < -band else 0))

        meta  = step.get("metadata") or {}
        is_ff = bool(meta.get("filtered_tool_failure", False))
        if _exclude_ff_global and is_ff:
            delta = _advance_state(cumulative_messages, action, step.get("observation"))
            if _compress_state_global:
                pending_delta.extend(delta)   # [BUG-H FIX]
            continue

        # ── State for this step  [BUG-H FIX] [ENG-L3] ────────────────────
        #
        # compress_state=True, not first labeled step:
        #   state = pending_delta (all messages since last labeled step,
        #   including any skipped gaps).  state_is_delta=True.
        #   Source-file state is NOT used here because it is a full
        #   snapshot, not a delta — mixing would break reconstruction.
        #
        # All other cases (first labeled step OR compress_state=False):
        #   state = source-file state if present [ML-M5], else cumulative.
        #   state_is_delta=False.
        if _compress_state_global and not first_labeled:
            step_state     = list(pending_delta)   # copy before clearing
            state_is_delta = True
        else:
            step_state     = step.get("state", list(cumulative_messages))
            state_is_delta = False

        pending_delta = []   # reset AFTER capturing above

        # [NEW-FIX] Normalize reasoning from multiple possible fields
        normalized_reasoning = normalize_reasoning(step)
        
        # [NEW-FIX] Calculate remaining steps for value estimation
        remaining_steps = n_steps - pos - 1
        
        labeled_steps.append({
            # Identity
            "task_id":             task_id,
            "repo":                repo,
            "trajectory_id":       trajectory_id,
            "model":               traj_model,
            "beam_id":             raw_beam_id_for_storage,
            "temperature":         temperature,
            "step_id":             step.get("step_id", pos),
            "step_position":       pos,
            "step_sequence_index": sequence_idx,   # [ML-CRITICAL-4] contiguous
            "remaining_steps":     remaining_steps,  # [NEW-FIX] for value estimation
            "iteration":           step.get("iteration", 0),
            # Content
            "action_type":         action_type,
            "tool_name":           action.get("tool_name") if action_type == "tool_call" else None,
            "tool_call_id":        action.get("tool_call_id"),
            # [BUG-K FIX] Bounds check removed.  content_hash_seq is built from
            # raw_steps; len matches; pos is always a valid index.
            "content_hash":        content_hash_seq[pos],
            # State  [BUG-H FIX]
            "state":               step_state,
            "state_is_delta":      state_is_delta,
            "action":              action,
            "observation":         step.get("observation"),
            "code":                step.get("code"),
            "content":             step.get("content"),
            "reasoning":           step.get("reasoning"),
            "reasoning_content":   step.get("reasoning_content"),
            # [NEW-FIX] Normalized reasoning for PRM training
            "reasoning_normalized": normalized_reasoning,
            # [NEW-FIX] Error type and reward (corrects compile/logic/tool confusion)
            "error_type":            error_type,
            "step_reward":           round(step_reward, 4),
            # LOO MC labels (Laplace-smoothed)
            "mc_label_soft":         round(mc_soft,  4),
            "mc_label_hard":         mc_hard,
            "mc_label_soft_normalized": round(mc_soft * length_norm_factor, 4),
            "mc_ci_low":             round(ci_low,   4),
            "mc_ci_high":            round(ci_high,  4),
            "mc_ci_valid":           ci_valid,           # [BUG-E FIX]
            "mc_reliable":           mc_reliable,
            "mc_zero_data":          mc_zero_data,
            "mc_label_prefix_soft":  round(pfx_soft, 4),
            "mc_label_prefix_hard":  pfx_hard,
            "mc_prefix_reliable":    mc_prefix_reliable,
            # Advantage  [ML-H4]
            "mc_advantage":          round(advantage, 4),
            "mc_advantage_normalized": round(advantage * length_norm_factor, 4),
            "mc_advantage_sign":     adv_sign,
            # Counts  [ML-M6]
            "mc_position_n_total":   loo_total,
            "mc_position_n_success": loo_succ,
            "mc_prefix_n_total":     pf_loo_total,
            "mc_prefix_n_success":   pf_loo_succ,
            # Outcome
            "outcome":               outcome,
            "outcome_score":         round(outcome_score, 4),
            "is_forced_final":       is_forced_final,
            "is_filtered_failure":   is_ff,
            "metadata":              meta if meta else None,
        })

        sequence_idx += 1
        first_labeled = False

        # Advance cumulative; seed pending_delta for next labeled step
        delta = _advance_state(cumulative_messages, action, step.get("observation"))
        if _compress_state_global:
            pending_delta.extend(delta)   # [BUG-H FIX]

    return (relative_path, labeled_steps)


# ─────────────────────────────────────────────────────────────────────────────
# Compact-file strict serializer  [ENG-H1]
# ─────────────────────────────────────────────────────────────────────────────

def _strict_json_default(obj):
    raise TypeError(
        f"Type {type(obj).__name__!r} not JSON-serializable: {obj!r}"
    )


# ─────────────────────────────────────────────────────────────────────────────
# Atomic output-dir creation  [ENG-M3]
# ─────────────────────────────────────────────────────────────────────────────

def safe_output_dir(path: Path) -> Path:
    for candidate in [path] + [Path(f"{path}_v{i}") for i in range(1, 1000)]:
        try:
            candidate.mkdir(parents=True, exist_ok=False)
            return candidate
        except FileExistsError:
            continue
    raise RuntimeError(f"No available output dir after 1000 attempts (base: {path})")


# ─────────────────────────────────────────────────────────────────────────────
# Dynamic chunksize  [ENG-M4]
# ─────────────────────────────────────────────────────────────────────────────

def optimal_chunksize(n_items: int, n_workers: int, multiplier: int = 4) -> int:
    return max(1, n_items // max(1, n_workers * multiplier))


# ─────────────────────────────────────────────────────────────────────────────
# Diagnostics  [ML-M8] [ML-L1] [ML-L6] [ML-M3]
# ─────────────────────────────────────────────────────────────────────────────

def print_dataset_diagnostics(
    rollout_metas: List[RolloutMeta],
    task_groups:   Dict[str, List[RolloutMeta]],
    stats:         IncrementalStats,
) -> None:
    print("\n" + "=" * 60)
    print("  Dataset Diagnostics")
    print("=" * 60)

    # [ML-L6] Per-task success rate distribution
    task_success_rates = [
        sum(1 for m in rollouts if m.success) / len(rollouts)
        for rollouts in task_groups.values()
        if rollouts
    ]
    if task_success_rates:
        n_all_fail    = sum(1 for r in task_success_rates if r == 0.0)
        n_all_succeed = sum(1 for r in task_success_rates if r == 1.0)
        n_useful      = len(task_success_rates) - n_all_fail - n_all_succeed
        sorted_rates  = sorted(task_success_rates)
        n             = len(sorted_rates)
        mean_rate     = sum(sorted_rates) / n
        median_rate   = sorted_rates[n // 2]
        print(f"\n  Per-task success rates:")
        print(f"    mean    = {mean_rate:.3f}")
        print(f"    median  = {median_rate:.3f}")
        print(f"    all-fail    (zero signal): {n_all_fail:>6,}")
        print(f"    all-succeed (zero signal): {n_all_succeed:>6,}")
        print(f"    mixed   (useful for PRM):  {n_useful:>6,}")

    # [ML-M3] Cross-model pooling validation
    model_stats: Dict[str, List[int]] = defaultdict(lambda: [0, 0])
    for r in rollout_metas:
        key = f"{r.provider}/{r.model}"
        model_stats[key][1] += 1
        if r.success:
            model_stats[key][0] += 1
    print(f"\n  Per-model success rates:")
    rates = []
    for model_key, (succ, total) in sorted(model_stats.items()):
        rate = succ / total if total else 0.0
        rates.append(rate)
        print(f"    {model_key:<45} {succ:>5}/{total:<6} = {rate:.3f}")
    if len(rates) > 1:
        spread = max(rates) - min(rates)
        if spread > 0.2:
            print(f"\n  ⚠️  WARNING: cross-model success rate spread = {spread:.3f} (>0.2)")
            print(f"     Pooling across models may corrupt MC estimates.")
            print(f"     Consider running per-model and merging labels.")

    # [ML-M8] Position-vs-score correlation
    pos_corr = stats.position_score_corr()
    if pos_corr is not None:
        print(f"\n  Position vs mc_label_soft correlation: {pos_corr:+.4f}")
        if abs(pos_corr) > 0.3:
            print(f"  ⚠️  Strong position bias (|corr|>0.3).")
            print(f"     Use mc_advantage for training instead of mc_label_soft.")

    # [ML-L1] Prefix-soft vs soft correlation — are they redundant?
    pfx_corr = stats.prefix_soft_corr()
    if pfx_corr is not None:
        print(f"\n  mc_label_prefix_soft vs mc_label_soft correlation: {pfx_corr:+.4f}")
        if abs(pfx_corr) > 0.95:
            print(f"  ⚠️  Very high correlation (|corr|>0.95).")
            print(f"     mc_label_prefix_soft adds no information.")
            print(f"     Recommend dropping it from training features.")

    # Advantage distribution  [ML-H4]
    if stats.n > 0:
        print(f"\n  Advantage distribution ({stats.n:,} labeled steps):")
        print(f"    positive (step helped):  {stats.adv_pos:>8,} "
              f"({100*stats.adv_pos/stats.n:.1f}%)")
        print(f"    negative (step hurt):    {stats.adv_neg:>8,} "
              f"({100*stats.adv_neg/stats.n:.1f}%)")
        print(f"    neutral:                 {stats.adv_zero:>8,} "
              f"({100*stats.adv_zero/stats.n:.1f}%)")

    print()


# ─────────────────────────────────────────────────────────────────────────────
# Entry point
# ─────────────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Parallel MC PRM Labeler v3",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--results_dir", type=Path,
        default=Path("RESULTS_ECML_NEW_MON_2_36_LATEST_FINAL_FIXED"),
    )
    parser.add_argument(
        "--output_dir", type=Path, default=Path("Mcts_group_labeled_ds"),
    )
    parser.add_argument(
        "--workers", type=int, default=None,
        help="Parallel workers (hard-capped at 128)",
    )
    parser.add_argument(
        "--min_rollouts", type=int, default=3,
        help="Min LOO-adjusted rollouts to keep a step",
    )
    parser.add_argument(
        "--success_threshold", type=float, default=1.0,
        help="[ML-CRITICAL-2] Min tests_pass_rate to count as success",
    )
    parser.add_argument(
        "--mc_hard_threshold", type=float, default=0.5,
        help="[ML-M1] Smoothed MC >= this → mc_label_hard=1",
    )
    parser.add_argument(
        "--laplace_alpha", type=float, default=1.0,
        help="[ML-L2] Add-alpha smoothing.  0=disable (unstable)",
    )
    parser.add_argument(
        "--exclude_filtered_failures", action="store_true",
    )
    parser.add_argument(
        "--soft_mc", action="store_true",
        help="[ML-NEW-4] Use continuous outcome_score as MC weight. "
             "Note: mc_ci_* fields are invalid in this mode (mc_ci_valid=False).",
    )
    parser.add_argument(
        "--advantage_neutral_band", type=float, default=0.05,
        help="[ML-NEW-M3] Advantage within ±band classified as neutral",
    )
    parser.add_argument(
        "--strict_mc", action="store_true",
        help="[BUG-G FIX / ML-H3] Abort when avg_beams_per_task < 10. "
             "Retroactive LOO approximation degrades significantly below "
             "this threshold.  Without this flag a warning is emitted instead.",
    )
    parser.add_argument(
        "--compress_state", action="store_true",
        help="[BUG-H FIX] Store state deltas instead of full snapshots. "
             "Reduces output size from O(steps²) to O(steps). "
             "First labeled step stores full state as reconstruction anchor; "
             "subsequent steps store only new messages (state_is_delta=True).",
    )
    parser.add_argument(
        "--length_normalize", action="store_true", default=True,
        help="Apply length normalization (1/sqrt(steps)) to MC returns. "
             "Reduces trajectory length bias.",
    )
    parser.add_argument(
        "--no_length_normalize", action="store_false", dest="length_normalize",
        help="Disable length normalization.",
    )
    parser.add_argument(
        "--reward_shaping", action="store_true", default=True,
        help="Enable step-type reward shaping (compile/test/tool signals). "
             "Provides dense learning signals for PRM.",
    )
    parser.add_argument(
        "--no_reward_shaping", action="store_false", dest="reward_shaping",
        help="Disable reward shaping (use only final outcome).",
    )
    parser.add_argument(
        "--limit", type=int, default=None,
        help="Process only first N files (debugging)",
    )
    parser.add_argument(
        "--verbose", action="store_true",
        help="Set log level to INFO",
    )
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.WARNING,
        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
        stream=sys.stderr,
    )
    if args.verbose:
        logging.getLogger().setLevel(logging.INFO)

    # Warn early on unstable settings
    if args.laplace_alpha == 0.0:
        logger.warning(
            "--laplace_alpha=0 disables smoothing.  Steps with 1–2 rollouts "
            "will receive exact 0.0 or 1.0 MC scores, which is numerically "
            "unstable for training.  Recommended: alpha >= 0.5.",
        )

    # [ENG-L4] Validate input before spinning up workers
    if not args.results_dir.exists():
        parser.error(f"--results_dir does not exist: {args.results_dir}")
    if not args.results_dir.is_dir():
        parser.error(f"--results_dir is not a directory: {args.results_dir}")

    # [ENG-M5] Worker cap
    requested_workers = args.workers or cpu_count()
    workers = min(requested_workers, 128)
    if requested_workers > 128:
        print(f"  ⚠️  Worker count capped at 128 (requested {requested_workers})")

    # [ENG-M3] Atomic output dir
    output_dir = safe_output_dir(args.output_dir)

    print(f"\n{'=' * 60}")
    print(f"  Parallel MC PRM Labeler v3")
    print(f"  Input              : {args.results_dir}")
    print(f"  Output             : {output_dir}")
    print(f"  Workers            : {workers}")
    print(f"  Success threshold  : {args.success_threshold}")
    print(f"  MC hard threshold  : {args.mc_hard_threshold}")
    print(f"  Min rollouts (LOO) : {args.min_rollouts}")
    print(f"  Laplace alpha      : {args.laplace_alpha}")
    print(f"  Soft MC            : {args.soft_mc}")
    print(f"  Advantage band     : ±{args.advantage_neutral_band}")
    print(f"  Strict MC          : {args.strict_mc}")
    print(f"  Compress state     : {args.compress_state}")
    print(f"  Length normalize   : {args.length_normalize}")
    print(f"  Reward shaping     : {args.reward_shaping}")
    print(f"  LOO mode           : on-the-fly subtraction (memory O(tasks×pos))")
    print(f"{'=' * 60}")

    # ── Pass 1 ────────────────────────────────────────────────────────────
    print("\n[Pass 1/2] Loading trajectory metadata...")
    files = sorted(args.results_dir.rglob("*_log_trj_repaired.json"))
    if args.limit:
        files = files[:args.limit]
        print(f"  Limited to {len(files):,} files (--limit)")
    print(f"  Found {len(files):,} trajectory files")

    if not files:
        print("  No files found. Exiting.")
        return

    load_args = [(f, args.results_dir, args.success_threshold) for f in files]
    cs1 = optimal_chunksize(len(files), workers)

    with Pool(workers) as pool:
        results = list(tqdm(
            pool.imap_unordered(load_and_build_meta, load_args, chunksize=cs1),
            total=len(files), desc="Loading metadata",
        ))

    rollout_metas = [r for r in results if r is not None]

    # Sort for deterministic output ordering across runs (imap_unordered is
    # non-deterministic; sorting by file_path makes the winner of any
    # trajectory_id collision the lexicographically first path).
    rollout_metas.sort(key=lambda m: m.file_path)

    # [BUG-A FIX] Enhanced dedup with detailed statistics (from PRM_TRAINING/scripts)
    # v2.5 attempted dedup inside worker processes via a function attribute;
    # each worker has its own memory space so cross-worker duplicates were
    # silently missed.  Doing it here is both correct and deterministic.
    print("\n  Deduplicating trajectory metadata...")
    
    seen_ids: Set[str] = set()
    deduped_metas: List[RolloutMeta] = []
    duplicate_stats: Dict[str, List[str]] = defaultdict(list)  # trajectory_id -> list of file paths
    
    for meta in rollout_metas:
        if meta.trajectory_id in seen_ids:
            duplicate_stats[meta.trajectory_id].append(meta.file_path)
            logger.warning(
                "Duplicate trajectory_id=%r (file: %s) — keeping the first "
                "occurrence (lexicographically earlier path) and dropping the "
                "second.  Both the MC stats and traj_meta will reflect only "
                "the kept copy (dedup runs before stat computation).  "
                "Remove or rename the duplicate file before re-running to "
                "avoid this warning.",
                meta.trajectory_id, meta.file_path,
            )
        else:
            seen_ids.add(meta.trajectory_id)
            deduped_metas.append(meta)
            # Initialize with the kept file path
            duplicate_stats[meta.trajectory_id] = [meta.file_path]

    n_dupes = len(rollout_metas) - len(deduped_metas)
    rollout_metas = deduped_metas
    
    # Enhanced duplicate reporting
    if n_dupes > 0:
        print(f"    Duplicates found: {n_dupes:,} trajectory files")
        print(f"    Unique trajectories kept: {len(deduped_metas):,}")
        
        # Show top duplicate groups for debugging
        multi_dupes = {tid: paths for tid, paths in duplicate_stats.items() if len(paths) > 1}
        if multi_dupes:
            print(f"    Trajectory IDs with multiple files: {len(multi_dupes):,}")
            # Show first few examples
            for i, (tid, paths) in enumerate(sorted(multi_dupes.items())[:3]):
                print(f"      {tid}: {len(paths)} files")
                for path in paths[:2]:  # Show first 2 paths
                    print(f"        - {path}")
                if len(paths) > 2:
                    print(f"        ... and {len(paths)-2} more")
            if len(multi_dupes) > 3:
                print(f"      ... and {len(multi_dupes)-3} more duplicate groups")
    else:
        print(f"    No duplicates found - all {len(deduped_metas):,} trajectories are unique")

    skipped = len(files) - len(rollout_metas) - n_dupes
    if skipped:
        print(f"  ⚠️  Skipped {skipped:,} unreadable files")
    if n_dupes:
        print(f"  ⚠️  Dropped {n_dupes:,} duplicate trajectory_ids (see log)")

    # [ML-NEW-5] Abort cleanly on empty result
    if not rollout_metas:
        logger.error(
            "All %d trajectory files failed to load. "
            "Check --results_dir path, file permissions, and JSON format.",
            len(files),
        )
        sys.exit(1)

    # [BUG-I FIX] Aggregate unknown action type warning — once, not per step
    all_unknown: Set[str] = set()
    for meta in rollout_metas:
        all_unknown.update(meta.unknown_action_types)
    if all_unknown:
        logger.warning(
            "Action types not in _ABBREV will map to 'X' in prefix "
            "fingerprints, making them indistinguishable from steps with "
            "missing action.type.  Found: %s.  "
            "Add entries to _ABBREV to distinguish them.",
            sorted(all_unknown),
        )

    # Group by task_id
    task_groups: Dict[str, List[RolloutMeta]] = defaultdict(list)
    for meta in rollout_metas:
        task_groups[meta.task_id].append(meta)

    n_tasks   = len(task_groups)
    n_trajs   = len(rollout_metas)
    avg_beams = n_trajs / max(n_tasks, 1)
    n_success = sum(1 for m in rollout_metas if m.success)

    print(f"\n  Tasks              : {n_tasks:,}")
    print(f"  Trajectories       : {n_trajs:,}")
    print(f"  Avg beams per task : {avg_beams:.1f}")
    print(f"  Successful trajs   : {n_success:,} / {n_trajs:,} "
          f"({100 * n_success / max(n_trajs, 1):.1f}%)")

    # [BUG-G FIX] --strict_mc: abort if beam count is too low for reliable LOO
    if avg_beams < 10.0:
        msg = (
            f"avg_beams_per_task = {avg_beams:.1f} < 10.  "
            f"Retroactive LOO approximation is unreliable at this scale.  "
            f"Collect more rollouts or remove --strict_mc to proceed with a warning."
        )
        if args.strict_mc:
            logger.error(msg)
            sys.exit(1)
        else:
            logger.warning(msg)

    # ── Compute GLOBAL MC stats per task (parallelized)  [BUG-F FIX / PERF-1]
    print("\n  Computing global MC statistics per task...")
    stats_args = [
        (task_id, rollouts, args.soft_mc)
        for task_id, rollouts in task_groups.items()
    ]
    cs_stats = optimal_chunksize(len(stats_args), workers)

    global_pos_by_task: Dict[str, _PosStats] = {}
    global_pfx_by_task: Dict[str, _PfxStats] = {}

    with Pool(workers) as pool:
        for task_id, pos_s, pfx_s in tqdm(
            pool.imap_unordered(
                _compute_task_mc_stats, stats_args, chunksize=cs_stats
            ),
            total=len(stats_args), desc="MC stats",
        ):
            global_pos_by_task[task_id] = pos_s
            global_pfx_by_task[task_id] = pfx_s

    # ── Compact traj metadata for LOO on-the-fly  ─────────────────────────
    # 3-tuple: (task_id, success, outcome_score).  action_type_seq excluded
    # — recomputed in worker from raw JSON; shipping it wastes pickling cost.
    traj_meta: Dict[str, Tuple[str, bool, float]] = {
        meta.trajectory_id: (meta.task_id, meta.success, meta.outcome_score)
        for meta in rollout_metas
    }

    # [BUG-B FIX] Store exact (numerator_sum, count) per task so workers can
    # compute the LOO base rate via direct subtraction — no multiply-back error.
    task_base_rate_stats: Dict[str, _BaseStats] = {
        task_id: (
            sum(m.outcome_score for m in rollouts) if args.soft_mc
            else float(sum(1 for m in rollouts if m.success)),
            len(rollouts),
        )
        for task_id, rollouts in task_groups.items()
        if rollouts
    }

    # ── Pass 2 ────────────────────────────────────────────────────────────
    print("\n[Pass 2/2] Labeling steps in parallel...")

    # [BUG-B FIX] 3-tuple args — task_base_rate and task_traj_count removed
    label_args = [
        (meta.file_path, meta.relative_path, meta.trajectory_id)
        for meta in rollout_metas
    ]
    cs2 = optimal_chunksize(len(label_args), workers)

    total_steps   = 0
    files_written = 0
    created_dirs: Set[Path] = set()
    diag_stats    = IncrementalStats()
    compact_path  = output_dir / "labeled_steps_compact.jsonl"

    with open(compact_path, "w") as f_compact, \
         Pool(
             workers,
             initializer=_init_worker,
             initargs=(
                 global_pos_by_task,
                 global_pfx_by_task,
                 traj_meta,
                 task_base_rate_stats,          # [BUG-B FIX]
                 args.min_rollouts,
                 args.exclude_filtered_failures,
                 args.mc_hard_threshold,
                 args.laplace_alpha,
                 args.success_threshold,
                 args.soft_mc,
                 args.advantage_neutral_band,
                 args.compress_state,           # [BUG-H FIX]
                 args.length_normalize,
                 args.reward_shaping,
             ),
         ) as pool:

        for relative_path, labeled_steps in tqdm(
            pool.imap_unordered(label_trajectory_file, label_args, chunksize=cs2),
            total=len(label_args), desc="Labeling",
        ):
            if not labeled_steps:
                continue

            output_path = output_dir / relative_path
            output_path = output_path.with_name(
                output_path.stem.replace("_repaired", "_labeled") + ".jsonl"
            )

            parent = output_path.parent
            if parent not in created_dirs:
                parent.mkdir(parents=True, exist_ok=True)
                created_dirs.add(parent)

            # Per-trajectory full JSONL
            with open(output_path, "w") as f_out:
                f_out.write(
                    "\n".join(json.dumps(s, default=str) for s in labeled_steps)
                    + "\n"
                )

            # [PERF-3] Compact JSONL — batch all steps for this trajectory
            # into one string before writing; reduces syscall count by
            # a factor of ~avg_steps_per_trajectory.
            compact_lines: List[str] = []
            for step in labeled_steps:
                # Extract provider from RolloutMeta (already available during labeling)
                provider = step["trajectory_id"].split("__")[0] if "__" in step["trajectory_id"] else "unknown"
                
                compact_record = {
                    # Identity
                    "task_id":               step["task_id"],
                    "repo":                  step["repo"],
                    "trajectory_id":         step["trajectory_id"],
                    "provider":              provider,
                    "model":                 step["model"],
                    "beam_id":               step["beam_id"],
                    "step_position":         step["step_position"],
                    "step_sequence_index":   step["step_sequence_index"],
                    "remaining_steps":       step["remaining_steps"],  # [NEW-FIX] for value estimation
                    # Action content (PRM training inputs)
                    "reasoning":             step["reasoning_normalized"],  # Normalized from multiple fields
                    "action_type":           step["action_type"],
                    "tool_name":             step["tool_name"],
                    "code":                  step["code"],
                    "content_hash":          step["content_hash"],
                    # Error signals
                    "error_type":            step["error_type"],
                    "step_reward":           step["step_reward"],
                    # MC value estimates
                    "mc_label_soft":         step["mc_label_soft"],
                    "mc_label_hard":         step["mc_label_hard"],
                    "mc_label_soft_normalized": step["mc_label_soft_normalized"],
                    "mc_advantage":          step["mc_advantage"],
                    "mc_advantage_normalized": step["mc_advantage_normalized"],
                    "mc_advantage_sign":     step["mc_advantage_sign"],
                    # Reliability filters
                    "mc_reliable":           step["mc_reliable"],
                    "mc_zero_data":          step["mc_zero_data"],
                    "mc_ci_low":             step["mc_ci_low"],
                    "mc_ci_high":            step["mc_ci_high"],
                    "mc_ci_valid":           step["mc_ci_valid"],
                    # Prefix signals (optional)
                    "mc_label_prefix_soft":  step["mc_label_prefix_soft"],
                    "mc_label_prefix_hard":  step["mc_label_prefix_hard"],
                    "mc_prefix_reliable":    step["mc_prefix_reliable"],
                    # Position statistics
                    "mc_position_n_total":   step["mc_position_n_total"],
                    "mc_position_n_success": step["mc_position_n_success"],
                    "mc_prefix_n_total":     step["mc_prefix_n_total"],
                    "mc_prefix_n_success":   step["mc_prefix_n_success"],
                    # Outcome
                    "outcome":               step["outcome"],
                    "outcome_score":         step["outcome_score"],
                    "is_forced_final":       step["is_forced_final"],
                    "is_filtered_failure":   step["is_filtered_failure"],
                }
                try:
                    compact_lines.append(
                        json.dumps(compact_record, default=_strict_json_default)
                    )
                except TypeError as exc:
                    logger.error(
                        "Non-serializable compact record traj=%s step=%s: %s",
                        step["trajectory_id"], step["step_position"], exc,
                    )

                diag_stats.update(
                    step_position = step["step_position"],
                    mc_soft       = step["mc_label_soft"],
                    pfx_soft      = step["mc_label_prefix_soft"],
                    advantage     = step["mc_advantage"],
                )

            if compact_lines:
                f_compact.write("\n".join(compact_lines) + "\n")

            total_steps   += len(labeled_steps)
            files_written += 1

    # ── Diagnostics ────────────────────────────────────────────────────────
    print_dataset_diagnostics(rollout_metas, task_groups, diag_stats)

    print(f"{'=' * 60}")
    print(f"  Final Statistics")
    print(f"{'=' * 60}")
    print(f"  Total labeled steps  : {total_steps:,}")
    print(f"  Files written        : {files_written:,}")
    print(f"  Unique tasks         : {n_tasks:,}")
    print(f"  Unique trajectories  : {n_trajs:,}")
    print(f"  Avg beams per task   : {avg_beams:.1f}")
    print(f"  Successful trajs     : {n_success:,} / {n_trajs:,} "
          f"({100 * n_success / max(n_trajs, 1):.1f}%)")
    print(f"  Output directory     : {output_dir}")
    print(f"  Compact labels       : {compact_path}")
    print(f"{'=' * 60}")
    print(f"\n✅  Done.")
    print(f"\nRecommended training signal: mc_advantage_normalized  (length-normalized)")
    print(f"Alternative signal:          mc_advantage  (not length-normalized)")
    print(f"Filter rows:                 mc_reliable == true  AND  mc_zero_data == false")
    if args.soft_mc:
        print(f"Note:                        mc_ci_valid=False in soft-MC mode; "
              f"ignore mc_ci_low / mc_ci_high")
    print(f"\n⚠️  NOTE [ML-H3]: Labels are RETROACTIVE (existing rollouts).")
    print(f"   True MC-PRM requires sampling new completions from each step.")
    print(f"   This approximation degrades below ~10 beams per task.")
    if args.compress_state:
        print(f"\nState reconstruction (--compress_state active):")
        print(f"   full_state(step_0)  = step.state                (state_is_delta=False)")
        print(f"   full_state(step_k)  = full_state(k-1) + step.state  (state_is_delta=True)")
    print(f"\nNext step:")
    print(f"   python hybrid_prm_labeler.py \\")
    print(f"       --input  {compact_path} \\")
    print(f"       --output prm_dataset/hybrid_labeled.jsonl")


if __name__ == "__main__":
    main()