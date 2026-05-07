#!/usr/bin/env python3
"""
Smoke Test for vLLM-based Hybrid Retrieval Pipeline

Tests:
1. vLLM endpoint connectivity (all 4 GPUs)
2. Embedding generation
3. BM25 indexing
4. Hybrid retrieval (BM25 + Dense)
5. Leakage prevention (target function masking)
6. End-to-end retrieval quality

Usage:
    python core/test_vllm_retrieval.py
    python core/test_vllm_retrieval.py --repo octree --ports 8000 8001 8002 8003
"""

import sys
import json
import argparse
import time
from pathlib import Path
import numpy as np

# Add parent directory to path
sys.path.append(str(Path(__file__).parent.parent))

from code_and_train_ds.t3_cpp.core.hybrid_retriever_vllm import (
    VLLMEmbeddingClient,
    HybridCodeRetriever,
    IdentifierAwareTokenizer
)


def print_section(title: str):
    """Print formatted section header"""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}\n")


def print_test(test_name: str, passed: bool, details: str = ""):
    """Print test result"""
    status = "✅ PASS" if passed else "❌ FAIL"
    print(f"{status} | {test_name}")
    if details:
        print(f"       {details}")


def test_1_vllm_connectivity(ports: list, host: str = "localhost"):
    """Test 1: Check vLLM endpoint connectivity"""
    print_section("TEST 1: vLLM Endpoint Connectivity")
    
    try:
        client = VLLMEmbeddingClient(base_ports=ports, host=host, timeout=10)
        
        # Check how many endpoints are available
        available_count = len(client.endpoints)
        expected_count = len(ports)
        
        passed = available_count > 0
        details = f"{available_count}/{expected_count} endpoints available"
        
        print_test("vLLM Connectivity", passed, details)
        
        if available_count < expected_count:
            print(f"\n⚠️  Warning: Only {available_count}/{expected_count} endpoints available")
            print(f"   Expected ports: {ports}")
            print(f"   Available: {client.endpoints}")
        
        return passed, client
    
    except Exception as e:
        print_test("vLLM Connectivity", False, f"Error: {str(e)}")
        return False, None


def test_2_embedding_generation(client: VLLMEmbeddingClient):
    """Test 2: Generate embeddings for sample code"""
    print_section("TEST 2: Embedding Generation")
    
    if client is None:
        print_test("Embedding Generation", False, "No client available")
        return False, None
    
    # Sample code snippets
    test_samples = [
        "double calculate_distance(const Point3D& p1, const Point3D& p2);",
        "class Point3D { double x, y, z; };",
        "std::vector<int> data;"
    ]
    
    try:
        start_time = time.time()
        embeddings = client.encode(test_samples, show_progress=False)
        elapsed = time.time() - start_time
        
        # Validate embeddings
        expected_shape = (len(test_samples), 4096)  # SFR-Embedding-2_R dimension
        shape_match = embeddings.shape == expected_shape
        
        # Check embeddings are not all zeros
        non_zero = np.any(embeddings != 0)
        
        # Check embeddings are normalized (cosine similarity ready)
        norms = np.linalg.norm(embeddings, axis=1)
        normalized = np.allclose(norms, 1.0, atol=0.1)
        
        passed = shape_match and non_zero
        details = f"Shape: {embeddings.shape}, Time: {elapsed:.2f}s, Normalized: {normalized}"
        
        print_test("Embedding Shape", shape_match, f"Expected {expected_shape}, Got {embeddings.shape}")
        print_test("Non-zero Embeddings", non_zero, f"Mean: {embeddings.mean():.4f}, Std: {embeddings.std():.4f}")
        print_test("Embedding Speed", True, f"{elapsed:.2f}s for {len(test_samples)} samples ({elapsed/len(test_samples):.3f}s per sample)")
        
        return passed, embeddings
    
    except Exception as e:
        print_test("Embedding Generation", False, f"Error: {str(e)}")
        return False, None


def test_3_identifier_tokenization():
    """Test 3: Identifier-aware tokenization"""
    print_section("TEST 3: Identifier-Aware Tokenization")
    
    tokenizer = IdentifierAwareTokenizer()
    
    test_cases = [
        ("calculateDistance", ["calculate", "distance", "calculatedistance"]),
        ("calculate_distance", ["calculate", "distance", "calculate_distance"]),
        ("std::vector", ["std", "vector", "std::vector"]),
        ("Point3D", ["point", "3d", "point3d"]),
    ]
    
    all_passed = True
    
    for input_text, expected_tokens in test_cases:
        tokens = tokenizer.tokenize(input_text)
        
        # Check if expected tokens are present (subset check)
        found = all(token in tokens for token in expected_tokens)
        
        print_test(f"Tokenize '{input_text}'", found, f"Tokens: {tokens[:5]}...")
        
        if not found:
            print(f"       Expected: {expected_tokens}")
            print(f"       Got: {tokens}")
            all_passed = False
    
    return all_passed


def test_4_bm25_indexing(repo_path: str):
    """Test 4: BM25 indexing"""
    print_section("TEST 4: BM25 Indexing")
    
    repo = Path(repo_path)
    if not repo.exists():
        print_test("BM25 Indexing", False, f"Repository not found: {repo_path}")
        return False, None
    
    try:
        # Create retriever (will build BM25 index)
        print("  Building BM25 index (this may take a moment)...")
        
        # Use a simple function_id for testing (won't mask anything if not found)
        retriever = HybridCodeRetriever(
            repo_path=str(repo),
            mask_function_id="test_function_id",
            vllm_ports=[8000, 8001, 8002, 8003]
        )
        
        # Validate index
        file_count = len(retriever.corpus)
        has_bm25 = retriever.bm25 is not None
        has_embeddings = retriever.file_embeddings is not None
        
        print_test("Files Indexed", file_count > 0, f"{file_count} files")
        print_test("BM25 Index Built", has_bm25, "BM25Okapi initialized")
        print_test("Embeddings Built", has_embeddings, f"Shape: {retriever.file_embeddings.shape if has_embeddings else 'None'}")
        
        passed = file_count > 0 and has_bm25 and has_embeddings
        
        return passed, retriever
    
    except Exception as e:
        print_test("BM25 Indexing", False, f"Error: {str(e)}")
        import traceback
        traceback.print_exc()
        return False, None


def test_5_hybrid_retrieval(retriever: HybridCodeRetriever):
    """Test 5: Hybrid retrieval (BM25 + Dense)"""
    print_section("TEST 5: Hybrid Retrieval")
    
    if retriever is None:
        print_test("Hybrid Retrieval", False, "No retriever available")
        return False
    
    # Test query
    query = "Point3D distance calculation Euclidean"
    
    try:
        print(f"  Query: '{query}'")
        print(f"  Retrieving top-5 files...\n")
        
        start_time = time.time()
        results = retriever.retrieve(
            query=query,
            top_k=5,
            stage1_candidates=30,
            exclude_target_file=False  # Don't exclude for this test
        )
        elapsed = time.time() - start_time
        
        # Validate results
        has_results = len(results) > 0
        
        print_test("Retrieval Success", has_results, f"Retrieved {len(results)} files in {elapsed:.2f}s")
        
        if has_results:
            print(f"\n  📊 Top-{len(results)} Retrieved Files:\n")
            
            for i, (file_path, content, score, breakdown) in enumerate(results, 1):
                rel_path = file_path.relative_to(retriever.repo_path)
                print(f"  {i}. {rel_path}")
                print(f"     RRF Score: {score:.4f}")
                print(f"     BM25: {breakdown['bm25']:.2f} | Dense: {breakdown['dense']:.4f}")
                print(f"     Preview: {content[:100].replace(chr(10), ' ')}...")
                print()
            
            # Check score ordering (should be descending)
            scores = [score for _, _, score, _ in results]
            properly_ordered = all(scores[i] >= scores[i+1] for i in range(len(scores)-1))
            
            print_test("Score Ordering", properly_ordered, f"Scores: {[f'{s:.4f}' for s in scores]}")
            
            # Check BM25 and Dense scores are both non-zero
            bm25_scores = [breakdown['bm25'] for _, _, _, breakdown in results]
            dense_scores = [breakdown['dense'] for _, _, _, breakdown in results]
            
            bm25_active = any(s > 0 for s in bm25_scores)
            dense_active = any(s > 0 for s in dense_scores)
            
            print_test("BM25 Active", bm25_active, f"Max BM25: {max(bm25_scores):.2f}")
            print_test("Dense Active", dense_active, f"Max Dense: {max(dense_scores):.4f}")
            
            return has_results and properly_ordered and bm25_active and dense_active
        
        return has_results
    
    except Exception as e:
        print_test("Hybrid Retrieval", False, f"Error: {str(e)}")
        import traceback
        traceback.print_exc()
        return False


def test_6_leakage_prevention(repo_path: str):
    """Test 6: Leakage prevention (target function masking)"""
    print_section("TEST 6: Leakage Prevention")
    
    repo = Path(repo_path)
    metadata_path = repo / "metadata.jsonl"
    
    if not metadata_path.exists():
        print_test("Leakage Prevention", False, f"Metadata not found: {metadata_path}")
        return False
    
    try:
        # Load first task
        with open(metadata_path) as f:
            task = json.loads(f.readline())
        
        function_id = task.get('function_id', '')
        namespace = task.get('namespace', '')
        
        print(f"  Testing with function: {namespace}")
        print(f"  Function ID: {function_id}\n")
        
        # Create retriever with masking
        retriever = HybridCodeRetriever(
            repo_path=str(repo),
            mask_function_id=function_id,
            vllm_ports=[8000, 8001, 8002, 8003]
        )
        
        # Retrieve with query expansion
        results = retriever.retrieve_with_query_expansion(
            task=task,
            top_k=5,
            exclude_target_file=True
        )
        
        # Check if target function body is in any retrieved file
        leakage_found = False
        
        for file_path, content, score, breakdown in results:
            # Check for unmasked function body (using actual marker format)
            if f"FUNCTION_ID: {function_id}" in content:
                # Check if it's properly masked
                if "// TODO: Implement this function" not in content:
                    # Check if it contains actual implementation
                    if "return" in content or "{" in content:
                        leakage_found = True
                        print(f"  ⚠️  Potential leakage in: {file_path.name}")
        
        print_test("Target File Excluded", len(results) > 0, f"Retrieved {len(results)} files")
        print_test("No Implementation Leakage", not leakage_found, "Function body properly masked/excluded")
        
        # Display retrieved files
        if results:
            print(f"\n  📁 Retrieved Files (target excluded):\n")
            for i, (file_path, content, score, breakdown) in enumerate(results, 1):
                rel_path = file_path.relative_to(repo)
                has_function_id = function_id in content
                print(f"  {i}. {rel_path} {'🔒 (masked)' if has_function_id else ''}")
        
        return not leakage_found
    
    except Exception as e:
        print_test("Leakage Prevention", False, f"Error: {str(e)}")
        import traceback
        traceback.print_exc()
        return False


def test_7_end_to_end_quality(repo_path: str):
    """Test 7: End-to-end retrieval quality"""
    print_section("TEST 7: End-to-End Retrieval Quality")
    
    repo = Path(repo_path)
    metadata_path = repo / "metadata.jsonl"
    
    if not metadata_path.exists():
        print_test("E2E Quality", False, f"Metadata not found: {metadata_path}")
        return False
    
    try:
        # Load multiple tasks
        tasks = []
        with open(metadata_path) as f:
            for i, line in enumerate(f):
                if i >= 3:  # Test first 3 tasks
                    break
                tasks.append(json.loads(line))
        
        print(f"  Testing retrieval quality on {len(tasks)} tasks...\n")
        
        quality_scores = []
        
        for i, task in enumerate(tasks, 1):
            namespace = task.get('namespace', '')
            function_id = task.get('function_id', '')
            
            print(f"  Task {i}: {namespace}")
            
            # Create retriever
            retriever = HybridCodeRetriever(
                repo_path=str(repo),
                mask_function_id=function_id,
                vllm_ports=[8000, 8001, 8002, 8003]
            )
            
            # Retrieve
            results = retriever.retrieve_with_query_expansion(
                task=task,
                top_k=5,
                exclude_target_file=True
            )
            
            if results:
                # Quality heuristics:
                # 1. At least 3 files retrieved
                # 2. Top result has high score
                # 3. Diversity in scores (not all the same)
                
                file_count = len(results)
                top_score = results[0][2] if results else 0
                scores = [score for _, _, score, _ in results]
                score_diversity = max(scores) - min(scores) if len(scores) > 1 else 0
                
                quality = (file_count >= 3) and (top_score > 0.01) and (score_diversity > 0.001)
                quality_scores.append(quality)
                
                print(f"    ✓ Retrieved {file_count} files, Top score: {top_score:.4f}, Diversity: {score_diversity:.4f}")
            else:
                quality_scores.append(False)
                print(f"    ✗ No files retrieved")
        
        # Overall quality
        avg_quality = sum(quality_scores) / len(quality_scores) if quality_scores else 0
        passed = avg_quality >= 0.66  # At least 2/3 tasks should have good quality
        
        print()
        print_test("Retrieval Quality", passed, f"{sum(quality_scores)}/{len(quality_scores)} tasks passed quality checks ({avg_quality*100:.0f}%)")
        
        return passed
    
    except Exception as e:
        print_test("E2E Quality", False, f"Error: {str(e)}")
        import traceback
        traceback.print_exc()
        return False


def main():
    parser = argparse.ArgumentParser(description="Smoke test for vLLM-based hybrid retrieval")
    parser.add_argument('--repo', default='benchmarks/octree', help='Repository path to test')
    parser.add_argument('--ports', nargs='+', type=int, default=[8000, 8001, 8002, 8003],
                       help='vLLM ports')
    parser.add_argument('--host', default='localhost', help='vLLM host')
    args = parser.parse_args()
    
    print("\n" + "="*70)
    print("  🧪 vLLM HYBRID RETRIEVAL SMOKE TEST")
    print("="*70)
    print(f"\n  Repository: {args.repo}")
    print(f"  vLLM Ports: {args.ports}")
    print(f"  vLLM Host: {args.host}")
    
    # Track results
    results = {}
    
    # Test 1: vLLM Connectivity
    passed, client = test_1_vllm_connectivity(args.ports, args.host)
    results['vLLM Connectivity'] = passed
    
    if not passed:
        print("\n❌ CRITICAL: vLLM endpoints not available. Cannot proceed.")
        print("\nTroubleshooting:")
        print("  1. Check if vLLM servers are running: screen -ls")
        print("  2. Test endpoints manually:")
        for port in args.ports:
            print(f"     curl http://{args.host}:{port}/v1/models")
        print("  3. Check vLLM logs: screen -r vllm_gpu0")
        sys.exit(1)
    
    # Test 2: Embedding Generation
    passed, embeddings = test_2_embedding_generation(client)
    results['Embedding Generation'] = passed
    
    # Test 3: Identifier Tokenization
    passed = test_3_identifier_tokenization()
    results['Identifier Tokenization'] = passed
    
    # Test 4: BM25 Indexing
    passed, retriever = test_4_bm25_indexing(args.repo)
    results['BM25 Indexing'] = passed
    
    if not passed:
        print("\n❌ CRITICAL: Indexing failed. Cannot proceed with retrieval tests.")
        sys.exit(1)
    
    # Test 5: Hybrid Retrieval
    passed = test_5_hybrid_retrieval(retriever)
    results['Hybrid Retrieval'] = passed
    
    # Test 6: Leakage Prevention
    passed = test_6_leakage_prevention(args.repo)
    results['Leakage Prevention'] = passed
    
    # Test 7: End-to-End Quality
    passed = test_7_end_to_end_quality(args.repo)
    results['E2E Quality'] = passed
    
    # Summary
    print_section("SUMMARY")
    
    total_tests = len(results)
    passed_tests = sum(results.values())
    
    for test_name, passed in results.items():
        status = "✅ PASS" if passed else "❌ FAIL"
        print(f"  {status} | {test_name}")
    
    print(f"\n  {'='*66}")
    print(f"  Total: {passed_tests}/{total_tests} tests passed ({passed_tests/total_tests*100:.0f}%)")
    print(f"  {'='*66}\n")
    
    if passed_tests == total_tests:
        print("  🎉 ALL TESTS PASSED! Pipeline is ready for production.")
        print("\n  Next steps:")
        print("    1. Run Condition B: python run_nvidia_api_single_pass.py --setting retrieved_context")
        print("    2. Compare with Condition A (baseline) and Condition D (agentic)")
        return 0
    else:
        print("  ⚠️  SOME TESTS FAILED. Please review errors above.")
        return 1


if __name__ == "__main__":
    sys.exit(main())
