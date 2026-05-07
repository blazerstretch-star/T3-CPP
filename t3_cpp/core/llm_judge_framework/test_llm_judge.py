#!/usr/bin/env python3
"""
Test script for LLM Judge integration.

Validates:
1. Persona routing logic
2. Cache functionality
3. JSON parsing
4. Score aggregation
"""

import json
import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent))

from llm_judge_personas import (
    route_evaluation_axes,
    get_axis_weights,
    aggregate_axis_scores,
    EVALUATION_AXES,
)


def test_axis_routing():
    """Test evaluation axis routing logic."""
    print("\n" + "=" * 60)
    print("TEST 1: Evaluation Axis Routing")
    print("=" * 60)
    
    # Test 1: Step with reasoning should get reasoning_coherence
    step = {"reasoning_normalized": "test reasoning", "tool_name": "search_repo_code", "step_position": 1}
    axes = route_evaluation_axes(step)
    assert "reasoning_coherence" in axes, f"Expected reasoning_coherence, got {axes}"
    print(f"✓ Step with reasoning → {axes}")
    
    # Test 2: Submission should get correctness
    step = {"tool_name": "submit_solution", "reasoning_normalized": "test", "step_position": 5}
    axes = route_evaluation_axes(step)
    assert "correctness" in axes, f"Expected correctness, got {axes}"
    print(f"✓ Submission step → {axes}")
    
    # Test 3: All steps (except step 0) should get action_appropriateness and efficiency
    step = {"tool_name": "read_file", "step_position": 2}
    axes = route_evaluation_axes(step)
    assert "action_appropriateness" in axes, f"Expected action_appropriateness, got {axes}"
    assert "efficiency" in axes, f"Expected efficiency, got {axes}"
    print(f"✓ All steps get action_appropriateness + efficiency → {axes}")
    
    # Test 4: Code generation should get correctness
    step = {"action_type": "generate_code", "reasoning_normalized": "test", "step_position": 3}
    axes = route_evaluation_axes(step)
    assert "correctness" in axes, f"Expected correctness for code gen, got {axes}"
    print(f"✓ Code generation → {axes}")
    
    # Test 5: Step 0 should NOT get action_appropriateness (no prior context)
    step_zero = {"step_position": 0, "tool_name": "search_repo_code", "reasoning_normalized": "test"}
    axes_zero = route_evaluation_axes(step_zero)
    assert "action_appropriateness" not in axes_zero, \
        "action_appropriateness should not fire at step 0 (no prior context)"
    print(f"✓ Step 0 routing excludes action_appropriateness: {axes_zero}")


def test_weight_adjustment():
    """Test dynamic weight adjustment."""
    print("\n" + "=" * 60)
    print("TEST 2: Dynamic Weight Adjustment")
    print("=" * 60)
    
    # Test 1: Default weights (equal)
    axes = ["correctness", "reasoning_coherence", "efficiency"]
    step = {"tool_name": "generate_code"}
    weights = get_axis_weights(axes, step)
    print(f"✓ Default weights: {weights}")
    assert abs(sum(weights.values()) - 1.0) < 0.01, "Weights should sum to 1.0"
    
    # Test 2: Submission upweights correctness
    step = {"tool_name": "submit_solution"}
    weights = get_axis_weights(axes, step)
    print(f"✓ Submission weights: {weights}")
    assert weights.get("correctness", 0) > 0.35, "Correctness should be upweighted"
    
    # Test 3: Error recovery upweights reasoning coherence (via step_reward)
    axes = ["reasoning_coherence", "action_appropriateness", "efficiency"]
    step_with_error = {
        "tool_name": "test_code_snippet",
        "step_reward": -0.75,  # What MC labeler produces on test failure
        "observation": {"error": "compilation failed"}
    }
    weights = get_axis_weights(axes, step_with_error)
    print(f"✓ Error recovery weights: {weights}")
    assert weights.get("reasoning_coherence", 0) > weights.get("efficiency", 0), \
        "Reasoning coherence should be upweighted over efficiency on error"
    assert weights.get("reasoning_coherence", 0) > 0.35, "Reasoning coherence should be >0.35 on error"





def test_score_aggregation():
    """Test score aggregation."""
    print("\n" + "=" * 60)
    print("TEST 3: Score Aggregation")
    print("=" * 60)
    
    # Test 1: Full coverage - all axes scored
    axis_scores = {
        "correctness": 0.9,
        "reasoning_coherence": 0.85,
        "efficiency": 0.8
    }
    
    weights = {
        "correctness": 0.4,
        "reasoning_coherence": 0.3,
        "efficiency": 0.3
    }
    
    result = aggregate_axis_scores(axis_scores, weights)
    
    assert isinstance(result, dict), "aggregate_axis_scores should return a dict"
    assert result["reliable"] == True, "Full coverage should be reliable"
    assert result["judge_overall_score"] is not None
    assert 0.0 <= result["judge_overall_score"] <= 1.0
    
    expected = 0.9 * 0.4 + 0.85 * 0.3 + 0.8 * 0.3
    assert abs(result["judge_overall_score"] - expected) < 0.01, f"Expected {expected}, got {result['judge_overall_score']}"
    print(f"✓ Full coverage: score={result['judge_overall_score']:.3f}, reliable={result['reliable']}, coverage={result['coverage']:.2f}")
    
    # Test 2: Partial coverage - one axis failed
    partial_scores = {"correctness": 0.9, "efficiency": 0.8}  # reasoning_coherence missing
    partial_result = aggregate_axis_scores(partial_scores, weights)
    assert partial_result["coverage"] < 1.0, "Partial scores should have coverage < 1.0"
    assert partial_result["coverage"] >= 0.6, "Should still meet min_coverage threshold"
    assert partial_result["reliable"] == True, "Should be reliable with 70% coverage"
    print(f"✓ Partial coverage: score={partial_result['judge_overall_score']:.3f}, coverage={partial_result['coverage']:.2f}")
    
    # Test 3: Below min_coverage threshold
    low_coverage_scores = {"efficiency": 0.8}  # Only 30% weight
    low_result = aggregate_axis_scores(low_coverage_scores, weights)
    assert low_result["reliable"] == False, "Should be unreliable with <60% coverage"
    assert low_result["judge_overall_score"] is None, "Should return None for unreliable aggregate"
    print(f"✓ Low coverage: reliable=False, score=None, coverage={low_result['coverage']:.2f}")
    
    # Test 4: Empty scores
    empty_result = aggregate_axis_scores({}, weights)
    assert empty_result["reliable"] == False
    assert empty_result["judge_overall_score"] is None
    assert empty_result["coverage"] == 0.0
    print(f"✓ Empty scores: reliable=False, score=None, coverage=0.0")


def test_cache_key_generation():
    """Test cache key generation."""
    print("\n" + "=" * 60)
    print("TEST 4: Cache Key Generation")
    print("=" * 60)
    
    from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_evaluator import compute_cache_key
    
    step1 = {
        "content_hash": "abc123",
        "action_type": "generate_code",
        "tool_name": "",
        "observation": {"result": "test"},
        "reasoning_normalized": "original reasoning"
    }
    
    step2 = {
        "content_hash": "abc123",
        "action_type": "generate_code",
        "tool_name": "",
        "observation": {"result": "test"},
        "reasoning_normalized": "original reasoning"
    }
    
    step3 = {
        "content_hash": "xyz789",
        "action_type": "generate_code",
        "tool_name": "",
        "observation": {"result": "test"},
        "reasoning_normalized": "original reasoning"
    }
    
    # Test 4: Same content_hash but different reasoning → different key
    step4 = {
        "content_hash": "abc123",
        "action_type": "generate_code",
        "tool_name": "",
        "observation": {"result": "test"},
        "reasoning_normalized": "completely different reasoning"
    }
    
    key1 = compute_cache_key(step1, "correctness", "gpt-4o-mini")
    key2 = compute_cache_key(step2, "correctness", "gpt-4o-mini")
    key3 = compute_cache_key(step3, "correctness", "gpt-4o-mini")
    key4 = compute_cache_key(step4, "correctness", "gpt-4o-mini")
    
    assert key1 == key2, "Identical steps should have same cache key"
    assert key1 != key3, "Different content_hash should have different cache keys"
    assert key1 != key4, "Different reasoning should produce different cache key"
    print(f"✓ Cache key 1 (original): {key1[:16]}...")
    print(f"✓ Cache key 2 (identical): {key2[:16]}...")
    print(f"✓ Cache key 3 (diff content): {key3[:16]}...")
    print(f"✓ Cache key 4 (diff reasoning): {key4[:16]}...")
    print(f"✓ Reasoning variation produces different cache key")


def test_hybrid_label_computation():
    """Test hybrid label computation."""
    print("\n" + "=" * 60)
    print("TEST 6: Hybrid Label Computation")
    print("=" * 60)
    
    from llm_judge_personas import compute_hybrid_label
    
    # Case 1: Both MC and judge reliable → hybrid blend
    step_full = {
        "mc_advantage_normalized": 0.15,
        "mc_reliable": True,
        "mc_zero_data": False,
    }
    judge_full = {"judge_overall_score": 0.8, "reliable": True}
    result = compute_hybrid_label(step_full, judge_full)
    assert result["label_source"] == "hybrid", f"Expected hybrid, got {result['label_source']}"
    assert result["include_in_training"] == True
    assert result["prm_training_label"] is not None
    print(f"✓ Full hybrid label: {result['prm_training_label']:.4f}, source={result['label_source']}")
    
    # Case 2: MC only reliable
    step_mc = {"mc_advantage_normalized": 0.10, "mc_reliable": True, "mc_zero_data": False}
    judge_fail = {"judge_overall_score": None, "reliable": False}
    result = compute_hybrid_label(step_mc, judge_fail)
    assert result["label_source"] == "mc_only", f"Expected mc_only, got {result['label_source']}"
    assert result["include_in_training"] == True
    assert result["prm_training_label"] == 0.10
    print(f"✓ MC-only label: {result['prm_training_label']:.4f}, source={result['label_source']}")
    
    # Case 3: Judge only reliable
    step_no_mc = {"mc_advantage_normalized": None, "mc_reliable": False, "mc_zero_data": True}
    judge_only = {"judge_overall_score": 0.75, "reliable": True}
    result = compute_hybrid_label(step_no_mc, judge_only)
    assert result["label_source"] == "judge_only", f"Expected judge_only, got {result['label_source']}"
    assert result["include_in_training"] == True
    print(f"✓ Judge-only label: {result['prm_training_label']:.4f}, source={result['label_source']}")
    
    # Case 4: Neither reliable → exclude
    step_bad = {"mc_advantage_normalized": None, "mc_reliable": False, "mc_zero_data": True}
    judge_bad = {"judge_overall_score": None, "reliable": False}
    result = compute_hybrid_label(step_bad, judge_bad)
    assert result["include_in_training"] == False
    assert result["prm_training_label"] is None
    assert result["label_source"] == "excluded"
    print(f"✓ Excluded step: include_in_training=False, label=None")
    
    # Case 5: Disagreement detection
    step_disagree = {"mc_advantage_normalized": 0.20, "mc_reliable": True, "mc_zero_data": False}
    judge_disagree = {"judge_overall_score": 0.2, "reliable": True}  # Judge thinks bad, MC thinks good
    result = compute_hybrid_label(step_disagree, judge_disagree)
    assert result["consensus"] == False, "Should detect disagreement"
    print(f"✓ Disagreement detected: consensus={result['consensus']}, label={result['prm_training_label']:.4f}")
    
    # Case 6: Agreement detection
    step_agree = {"mc_advantage_normalized": 0.15, "mc_reliable": True, "mc_zero_data": False}
    judge_agree = {"judge_overall_score": 0.75, "reliable": True}  # Both positive
    result = compute_hybrid_label(step_agree, judge_agree)
    assert result["consensus"] == True, "Should detect agreement"
    print(f"✓ Agreement detected: consensus={result['consensus']}, label={result['prm_training_label']:.4f}")


async def test_async_semaphore():
    """Verify no deadlock with nested semaphores."""
    import asyncio
    
    traj_semaphore = asyncio.Semaphore(2)
    api_semaphore = asyncio.Semaphore(6)
    
    async def mock_api_call(i):
        async with api_semaphore:
            await asyncio.sleep(0.01)  # Simulate API latency
            return i * 2
    
    async def process_trajectory(traj_id, n_steps):
        async with traj_semaphore:
            tasks = [mock_api_call(i) for i in range(n_steps)]
            step_results = await asyncio.gather(*tasks)
            return traj_id, step_results
    
    trajectories = {f"traj_{i}": 5 for i in range(5)}  # 5 trajectories, 5 steps each
    tasks = [process_trajectory(tid, n) for tid, n in trajectories.items()]
    
    all_results = await asyncio.gather(*tasks)
    assert len(all_results) == 5, "All trajectories should complete"
    return all_results


def test_no_async_deadlock():
    """Test async semaphore handling."""
    print("\n" + "=" * 60)
    print("TEST 7: Async Semaphore (No Deadlock)")
    print("=" * 60)
    
    import asyncio
    results = asyncio.run(test_async_semaphore())
    print(f"✓ Async semaphore: {len(results)} trajectories processed without deadlock")
    print(f"✓ Each trajectory completed 5 API calls")


def test_json_extraction():
    """Test JSON extraction from LLM responses."""
    print("\n" + "=" * 60)
    print("TEST 5: JSON Extraction")
    print("=" * 60)
    
    from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_evaluator import extract_json_from_response
    import json as json_module
    
    # Test 1: Clean JSON
    response1 = '{"score": 0.85, "justification": "Good"}'
    result1 = extract_json_from_response(response1)
    assert result1["score"] == 0.85
    print("✓ Clean JSON extraction")
    
    # Test 2: JSON in markdown
    response2 = '''Here's my evaluation:
```json
{"score": 0.75, "justification": "Okay"}
```
Hope this helps!'''
    result2 = extract_json_from_response(response2)
    assert result2["score"] == 0.75
    print("✓ Markdown JSON extraction")
    
    # Test 3: JSON with extra text
    response3 = 'Based on my analysis: {"score": 0.90, "justification": "Excellent"} as shown above.'
    result3 = extract_json_from_response(response3)
    assert result3["score"] == 0.90
    print("✓ JSON with surrounding text extraction")
    
    # Test 4: Adversarial - non-JSON braces before real JSON
    response4 = 'The step {description} used {"correctness_score": 0.85, "justification": "Good"}'
    result4 = extract_json_from_response(response4)
    assert result4.get("correctness_score") == 0.85, \
        f"Should find second JSON object, got: {result4}"
    print("✓ JSON with earlier non-JSON braces extraction")
    
    # Test 5: Nested JSON objects
    response5 = '{"score": 0.80, "details": {"sub_score": 0.9}, "justification": "Good"}'
    result5 = extract_json_from_response(response5)
    assert result5["score"] == 0.80
    assert result5["details"]["sub_score"] == 0.9
    print("✓ Nested JSON extraction")
    
    # Test 6: Extraction failure raises JSONDecodeError
    try:
        extract_json_from_response("No JSON here at all")
        assert False, "Should have raised JSONDecodeError"
    except json_module.JSONDecodeError:
        print("✓ Invalid response raises JSONDecodeError correctly")


def run_test(test_fn):
    """Run a test with clean state and timing."""
    import time
    start = time.time()
    try:
        test_fn()
        elapsed = time.time() - start
        print(f"  ({elapsed*1000:.0f}ms)")
        return True
    except AssertionError as e:
        print(f"\n  ❌ FAILED: {e}")
        return False
    except Exception as e:
        import traceback
        print(f"\n  ❌ ERROR: {e}")
        traceback.print_exc()
        return False


def main():
    """Run all tests."""
    print("\n" + "=" * 60)
    print("  LLM Judge Integration Tests")
    print("=" * 60)
    
    tests = [
        test_axis_routing,
        test_weight_adjustment,
        test_score_aggregation,
        test_cache_key_generation,
        test_json_extraction,
        test_hybrid_label_computation,
        test_no_async_deadlock,
    ]
    
    results = [run_test(t) for t in tests]
    passed = sum(results)
    
    print("\n" + "=" * 60)
    if all(results):
        print(f"  ✅ ALL {len(tests)} TESTS PASSED")
    else:
        print(f"  ⚠️  {passed}/{len(tests)} tests passed")
    print("=" * 60 + "\n")
    
    if all(results):
        print("Next steps:")
        print("1. Install dependencies: pip install -r requirements_llm_judge.txt")
        print("2. Set API key: export OPENAI_API_KEY='sk-...'")
        print("3. Run on sample data:")
        print("   python add_llm_judge.py \\")
        print("       --input Mcts_group_labeled_ds/labeled_steps_compact.jsonl \\")
        print("       --output test_judged.jsonl \\")
        print("       --judge_model gpt-4o-mini \\")
        print("       --sample_rate 0.01  # Test on 1% first")
    
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())
