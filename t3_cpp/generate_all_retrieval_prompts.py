#!/usr/bin/env python3
"""
Generate retrieval-augmented prompts for all repositories with comprehensive configuration tracking.
Stores all configuration details for paper reporting.
"""

import os
import json
import time
import subprocess
from pathlib import Path
from datetime import datetime

# Configuration for paper
RETRIEVAL_CONFIG = {
    "retrieval_method": "hybrid",
    "hybrid_weights": {
        "bm25_weight": 0.3,
        "dense_weight": 0.7
    },
    "retrieval_parameters": {
        "top_k": 5,
        "stage1_candidates": 100,
        "min_code_ratio": 0.3
    },
    "file_type_weights": {
        "implementation": 1.0,
        "header": 1.5,
        "test": 0.3
    },
    "embedding_model": {
        "name": "Qwen/Qwen3-Embedding-8B",
        "dimension": 4096,
        "endpoints": [
            "http://192.168.1.100:8001/v1",
            "http://192.168.1.100:8002/v1",
            "http://192.168.1.100:8003/v1",
            "http://192.168.1.100:8004/v1"
        ]
    },
    "bm25_parameters": {
        "k1": 1.5,
        "b": 0.75
    }
}

REPOSITORIES = [
    "adhesion", "allocator_builder", "bplustree", "cg3lib", "cgal_kernel",
    "cpp-projects", "cppqed", "distlib", "hospital", "huffman",
    "jemalloc_utils", "leveldb", "littlefs", "molecular_dynamics",
    "movie_recommendation", "nanort", "numerical_methods", "octree",
    "pbrt-v3", "prepair", "route_planning", "sph", "suffixtree", "tinyrenderer"
]

def generate_prompts_for_repo(repo_name, output_base="ablation_prompts"):
    """Generate retrieval-augmented prompts for a single repository."""
    output_dir = f"{output_base}/{repo_name}/retrieval"
    
    cmd = [
        "python", "scripts/generate_prompts_with_retrieval.py",
        "--repo", repo_name,
        "--setting", "retrieval",
        "--output_dir", output_dir,
        "--top_k", "5",
        "--stage1_candidates", "100",
        "--min_code_ratio", "0.3",
        "--boost_header", "1.5",
        "--test_weight", "0.3",
        "--vllm_ports", "8000", "8001", "8002", "8003"  # Correct vLLM ports (localhost)
    ]
    
    print(f"\n{'='*80}")
    print(f"Generating prompts for: {repo_name}")
    print(f"{'='*80}")
    
    start_time = time.time()
    result = subprocess.run(cmd, capture_output=True, text=True)
    elapsed_time = time.time() - start_time
    
    success = result.returncode == 0
    
    return {
        "repo": repo_name,
        "success": success,
        "elapsed_time": elapsed_time,
        "output_dir": output_dir,
        "stdout": result.stdout,
        "stderr": result.stderr
    }

def analyze_prompts_file(prompts_file):
    """Analyze generated prompts file for statistics."""
    if not os.path.exists(prompts_file):
        return None
    
    with open(prompts_file, 'r') as f:
        prompts = [json.loads(line) for line in f]
    
    total_functions = len(prompts)
    # Fix: use 'retrieval_info' instead of 'retrieved_contexts'
    total_contexts = sum(p.get('retrieval_info', {}).get('top_k', 0) for p in prompts)
    
    # Calculate prompt sizes
    prompt_sizes = []
    for p in prompts:
        prompt_text = p.get('prompt', '')
        prompt_sizes.append(len(prompt_text))
    
    # Analyze retrieved file types
    file_types = {"test": 0, "header": 0, "implementation": 0}
    for p in prompts:
        # Fix: use 'retrieval_info' -> 'retrieved_files'
        retrieved_files = p.get('retrieval_info', {}).get('retrieved_files', [])
        for file_path in retrieved_files:
            if 'test' in file_path.lower():
                file_types["test"] += 1
            elif file_path.endswith('.h') or file_path.endswith('.hpp'):
                file_types["header"] += 1
            else:
                file_types["implementation"] += 1
    
    return {
        "total_functions": total_functions,
        "total_contexts": total_contexts,
        "avg_contexts_per_function": total_contexts / total_functions if total_functions > 0 else 0,
        "prompt_size_stats": {
            "min_chars": min(prompt_sizes) if prompt_sizes else 0,
            "max_chars": max(prompt_sizes) if prompt_sizes else 0,
            "avg_chars": sum(prompt_sizes) / len(prompt_sizes) if prompt_sizes else 0,
            "avg_tokens_estimate": sum(prompt_sizes) / len(prompt_sizes) / 4 if prompt_sizes else 0
        },
        "file_type_distribution": file_types,
        "file_type_percentages": {
            k: (v / total_contexts * 100) if total_contexts > 0 else 0
            for k, v in file_types.items()
        }
    }

def main():
    print("="*80)
    print("RETRIEVAL-AUGMENTED PROMPT GENERATION FOR ALL REPOSITORIES")
    print("="*80)
    print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Total repositories: {len(REPOSITORIES)}")
    print()
    
    # Store results
    results = []
    overall_start = time.time()
    
    # Generate prompts for each repository
    for i, repo in enumerate(REPOSITORIES, 1):
        print(f"\n[{i}/{len(REPOSITORIES)}] Processing: {repo}")
        result = generate_prompts_for_repo(repo)
        results.append(result)
        
        if result["success"]:
            print(f"✅ Success in {result['elapsed_time']:.2f}s")
        else:
            print(f"❌ Failed in {result['elapsed_time']:.2f}s")
            print(f"Error: {result['stderr'][:200]}")
    
    overall_elapsed = time.time() - overall_start
    
    # Analyze all generated prompts
    print("\n" + "="*80)
    print("ANALYZING GENERATED PROMPTS")
    print("="*80)
    
    repo_statistics = {}
    for result in results:
        if result["success"]:
            prompts_file = f"{result['output_dir']}/prompts.jsonl"
            stats = analyze_prompts_file(prompts_file)
            if stats:
                repo_statistics[result["repo"]] = stats
                print(f"\n{result['repo']}:")
                print(f"  Functions: {stats['total_functions']}")
                print(f"  Avg contexts: {stats['avg_contexts_per_function']:.1f}")
                print(f"  Avg prompt size: {stats['prompt_size_stats']['avg_chars']:.0f} chars (~{stats['prompt_size_stats']['avg_tokens_estimate']:.0f} tokens)")
    
    # Create comprehensive report
    report = {
        "metadata": {
            "generation_date": datetime.now().isoformat(),
            "total_repositories": len(REPOSITORIES),
            "successful_repositories": sum(1 for r in results if r["success"]),
            "failed_repositories": sum(1 for r in results if not r["success"]),
            "total_elapsed_time": overall_elapsed
        },
        "configuration": RETRIEVAL_CONFIG,
        "repository_results": results,
        "repository_statistics": repo_statistics,
        "aggregate_statistics": {
            "total_functions": sum(s["total_functions"] for s in repo_statistics.values()),
            "total_contexts_retrieved": sum(s["total_contexts"] for s in repo_statistics.values()),
            "avg_prompt_size_chars": sum(s["prompt_size_stats"]["avg_chars"] for s in repo_statistics.values()) / len(repo_statistics) if repo_statistics else 0,
            "avg_prompt_size_tokens": sum(s["prompt_size_stats"]["avg_tokens_estimate"] for s in repo_statistics.values()) / len(repo_statistics) if repo_statistics else 0
        }
    }
    
    # Save comprehensive report
    report_file = "ablation_prompts/GENERATION_REPORT.json"
    os.makedirs(os.path.dirname(report_file), exist_ok=True)
    with open(report_file, 'w') as f:
        json.dump(report, f, indent=2)
    
    # Create paper-ready summary
    paper_summary = {
        "title": "Retrieval-Augmented Code Generation Configuration",
        "retrieval_method": "Hybrid (BM25 + Dense Embeddings)",
        "embedding_model": "Qwen/Qwen3-Embedding-8B (4096-dim)",
        "retrieval_parameters": RETRIEVAL_CONFIG["retrieval_parameters"],
        "file_type_boosting": RETRIEVAL_CONFIG["file_type_weights"],
        "dataset_statistics": {
            "total_repositories": report["metadata"]["total_repositories"],
            "total_functions": report["aggregate_statistics"]["total_functions"],
            "total_contexts_retrieved": report["aggregate_statistics"]["total_contexts_retrieved"],
            "avg_contexts_per_function": report["aggregate_statistics"]["total_contexts_retrieved"] / report["aggregate_statistics"]["total_functions"] if report["aggregate_statistics"]["total_functions"] > 0 else 0,
            "avg_prompt_size_tokens": report["aggregate_statistics"]["avg_prompt_size_tokens"]
        }
    }
    
    paper_file = "ablation_prompts/PAPER_SUMMARY.json"
    with open(paper_file, 'w') as f:
        json.dump(paper_summary, f, indent=2)
    
    # Print final summary
    print("\n" + "="*80)
    print("GENERATION COMPLETE")
    print("="*80)
    print(f"Total time: {overall_elapsed:.2f}s")
    print(f"Successful: {report['metadata']['successful_repositories']}/{report['metadata']['total_repositories']}")
    print(f"Total functions: {report['aggregate_statistics']['total_functions']}")
    print(f"Total contexts: {report['aggregate_statistics']['total_contexts_retrieved']}")
    print(f"Avg prompt size: ~{report['aggregate_statistics']['avg_prompt_size_tokens']:.0f} tokens")
    print(f"\nReports saved:")
    print(f"  - {report_file}")
    print(f"  - {paper_file}")

if __name__ == "__main__":
    main()
