#!/usr/bin/env python3
"""
Test script for plausible-but-wrong detection system.

Validates that all components work correctly:
1. Prompt enhancements
2. Detection function
3. Adaptive strategy integration
4. Full pipeline with flagging
"""

import sys
from code_and_train_ds.t3_cpp.core.llm_judge_framework.consensus_labeling import (
    detect_plausible_but_wrong,
    AdaptiveStrategy,
    apply_consensus_labeling
)
from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_calibrated import EVALUATION_AXES


def test_prompts():
    """Test that prompts contain observation consistency guidance."""
    print("=" * 60)
    print("TEST 1: Prompt Enhancements")
    print("=" * 60)
    
    correctness_prompt = EVALUATION_AXES['correctness']['system_prompt']
    process_prompt = EVALUATION_AXES['process_quality']['system_prompt']
    
    checks = [
        ("Correctness: Observation consistency", "Observation consistency" in correctness_prompt),
        ("Correctness: Plausible but Wrong", "Plausible but Wrong" in correctness_prompt),
        ("Process: Observation consistency", "Observation consistency" in process_prompt),
        ("Process: Plausible but Wrong", "Plausible but Wrong" in process_prompt),
    ]
    
    all_passed = True
    for name, result in checks:
        status = "✓" if result else "✗"
        print(f"  {status} {name}")
        if not result:
            all_passed = False
    
    print(f"\nResult: {'PASS' if all_passed else 'FAIL'}\n")
    return all_passed


def test_detection_function():
    """Test the detect_plausible_but_wrong function."""
    print("=" * 60)
    print("TEST 2: Detection Function")
    print("=" * 60)
    
    test_cases = [
        {
            "name": "Strong signal (judge optimistic + MC pessimistic + error)",
            "step": {
                'judge_overall_score': 0.75,
                'mc_label_soft': 0.25,
                'observation': {'status': 'compile_error', 'exit_code': 1},
                'error_type': 'compile_error'
            },
            "expected_detected": True,
            "expected_confidence_min": 0.7
        },
        {
            "name": "Medium signal (judge optimistic + error, no MC)",
            "step": {
                'judge_overall_score': 0.70,
                'mc_label_soft': 0.50,
                'observation': {'status': 'timeout'},
                'error_type': 'timeout'
            },
            "expected_detected": True,
            "expected_confidence_min": 0.5
        },
        {
            "name": "No signal (judge pessimistic)",
            "step": {
                'judge_overall_score': 0.30,
                'mc_label_soft': 0.25,
                'observation': {'status': 'error'},
            },
            "expected_detected": False,
            "expected_confidence_min": 0.0
        },
        {
            "name": "Weak signal (disagreement without error)",
            "step": {
                'judge_overall_score': 0.75,
                'mc_label_soft': 0.25,
                'observation': {'status': 'success', 'exit_code': 0},
            },
            "expected_detected": True,  # Weak signal still detected
            "expected_confidence_min": 0.3  # But with low confidence
        }
    ]
    
    all_passed = True
    for test in test_cases:
        result = detect_plausible_but_wrong(test["step"])
        detected = result["is_plausible_but_wrong"]
        confidence = result["confidence"]
        
        passed = (
            detected == test["expected_detected"] and
            confidence >= test["expected_confidence_min"]
        )
        
        status = "✓" if passed else "✗"
        print(f"  {status} {test['name']}")
        print(f"      Detected: {detected} (expected: {test['expected_detected']})")
        print(f"      Confidence: {confidence:.2f} (min: {test['expected_confidence_min']})")
        
        if not passed:
            all_passed = False
    
    print(f"\nResult: {'PASS' if all_passed else 'FAIL'}\n")
    return all_passed


def test_adaptive_strategy():
    """Test AdaptiveStrategy integration."""
    print("=" * 60)
    print("TEST 3: Adaptive Strategy Integration")
    print("=" * 60)
    
    strategy = AdaptiveStrategy()
    
    # Plausible-but-wrong case
    pbw_step = {
        'judge_overall_score': 0.75,
        'mc_label_soft': 0.25,
        'mc_advantage_normalized': -0.5,
        'mc_reliable': True,
        'mc_zero_data': False,
        'mc_n_rollouts': 10,
        'observation': {'status': 'compile_error', 'exit_code': 1},
        'error_type': 'compile_error'
    }
    
    result = strategy.compute_consensus_label(pbw_step)
    
    if result is None:
        print("  ✗ Strategy returned None")
        print("\nResult: FAIL\n")
        return False
    
    consensus_label, confidence, metadata = result
    
    checks = [
        ("Returns result", result is not None),
        ("Rule is plausible_but_wrong_detected", metadata.get("rule") == "plausible_but_wrong_detected"),
        ("Alpha (MC weight) is 0.9", metadata.get("alpha") == 0.9),
        ("Has pbw_confidence", "pbw_confidence" in metadata),
        ("Consensus label is negative", consensus_label < 0),  # MC is negative
    ]
    
    all_passed = True
    for name, result in checks:
        status = "✓" if result else "✗"
        print(f"  {status} {name}")
        if not result:
            all_passed = False
    
    print(f"\n  Consensus label: {consensus_label:.3f}")
    print(f"  Confidence: {confidence}")
    print(f"  Alpha: {metadata.get('alpha')}")
    
    print(f"\nResult: {'PASS' if all_passed else 'FAIL'}\n")
    return all_passed


def test_full_pipeline():
    """Test full consensus labeling pipeline."""
    print("=" * 60)
    print("TEST 4: Full Pipeline with Flagging")
    print("=" * 60)
    
    strategy = AdaptiveStrategy()
    
    test_steps = [
        # Plausible-but-wrong case (should be flagged)
        {
            'judge_overall_score': 0.75,
            'mc_label_soft': 0.25,
            'mc_advantage_normalized': -0.5,
            'mc_reliable': True,
            'mc_zero_data': False,
            'mc_n_rollouts': 10,
            'observation': {'status': 'compile_error', 'exit_code': 1},
            'error_type': 'compile_error'
        },
        # Normal case (should not be flagged)
        {
            'judge_overall_score': 0.65,
            'mc_label_soft': 0.60,
            'mc_advantage_normalized': 0.2,
            'mc_reliable': True,
            'mc_zero_data': False,
            'mc_n_rollouts': 5,
            'observation': {'status': 'success', 'exit_code': 0}
        }
    ]
    
    labeled = apply_consensus_labeling(test_steps, strategy, min_confidence=0.0)
    
    checks = [
        ("Output has 2 steps", len(labeled) == 2),
        ("Step 1 has prm_label", "prm_label" in labeled[0]),
        ("Step 1 has plausible_but_wrong", "plausible_but_wrong" in labeled[0]),
        ("Step 1 is detected as PBW", labeled[0].get("plausible_but_wrong", {}).get("is_plausible_but_wrong", False)),
        ("Step 1 needs review", labeled[0].get("needs_human_review", False)),
        ("Step 2 is NOT detected as PBW", not labeled[1].get("plausible_but_wrong", {}).get("is_plausible_but_wrong", True)),
        ("Step 2 does NOT need review", not labeled[1].get("needs_human_review", False)),
    ]
    
    all_passed = True
    for name, result in checks:
        status = "✓" if result else "✗"
        print(f"  {status} {name}")
        if not result:
            all_passed = False
    
    print(f"\n  Step 1 PRM label: {labeled[0].get('prm_label', 'N/A')}")
    print(f"  Step 1 PBW confidence: {labeled[0].get('plausible_but_wrong', {}).get('confidence', 'N/A')}")
    print(f"  Step 2 PRM label: {labeled[1].get('prm_label', 'N/A')}")
    
    print(f"\nResult: {'PASS' if all_passed else 'FAIL'}\n")
    return all_passed


def main():
    """Run all tests."""
    print("\n" + "=" * 60)
    print("PLAUSIBLE-BUT-WRONG DETECTION SYSTEM TEST SUITE")
    print("=" * 60 + "\n")
    
    results = []
    
    results.append(("Prompt Enhancements", test_prompts()))
    results.append(("Detection Function", test_detection_function()))
    results.append(("Adaptive Strategy", test_adaptive_strategy()))
    results.append(("Full Pipeline", test_full_pipeline()))
    
    print("=" * 60)
    print("SUMMARY")
    print("=" * 60)
    
    for name, passed in results:
        status = "✓ PASS" if passed else "✗ FAIL"
        print(f"  {status}: {name}")
    
    all_passed = all(passed for _, passed in results)
    
    print("\n" + "=" * 60)
    if all_passed:
        print("ALL TESTS PASSED ✓")
    else:
        print("SOME TESTS FAILED ✗")
    print("=" * 60 + "\n")
    
    return 0 if all_passed else 1


if __name__ == "__main__":
    sys.exit(main())
