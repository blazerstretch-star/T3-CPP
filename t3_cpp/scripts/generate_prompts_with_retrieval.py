#!/usr/bin/env python3
"""
Generate prompts with retrieval-augmented context for Condition B evaluation.

Performance: ~1.5s for first function (build index), ~0.05s per subsequent function.
Example: 100 functions = 1.5s + (99 × 0.05s) = ~6.5s total
"""

import sys
import json
import argparse
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from code_and_train_ds.t3_cpp.core.hybrid_retriever_vllm import HybridCodeRetriever


def generate_prompts_with_retrieval(
    repo: str,
    setting: str,
    output_dir: Path,
    top_k: int = 5,
    max_chars_per_result: int = 5000,
    vllm_ports: list = None,
    file_type_weights: dict = None,
    exclude_test_files: bool = False,
    min_code_ratio: float = 0.0,
    stage1_candidates: int = 100
):
    """Generate retrieval-augmented prompts for all functions in a repository.
    
    Args:
        repo: Repository name (e.g., "octree")
        setting: Setting name (e.g., "baseline_retrieval")
        output_dir: Output directory for prompts.jsonl
        top_k: Number of retrieved contexts per function
        max_chars_per_result: Max characters per retrieved file
        vllm_ports: vLLM server ports (default: [8000, 8001, 8002, 8003])
        file_type_weights: Boost scores by file type (e.g., {'cpp': 1.5, 'test': 0.5})
        exclude_test_files: Completely exclude test files from results
        min_code_ratio: Minimum code-to-comment ratio (0.0-1.0)
        stage1_candidates: Number of BM25 candidates (default: 100 for diversity)
    """
    
    if vllm_ports is None:
        vllm_ports = [8000, 8001, 8002, 8003]
    
    # Setup paths
    repo_path = Path("benchmarks") / repo
    metadata_path = repo_path / "metadata.jsonl"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    if not metadata_path.exists():
        print(f"❌ Metadata not found: {metadata_path}")
        sys.exit(1)
    
    # Load metadata
    print(f"📖 Loading metadata from {metadata_path}...")
    with open(metadata_path) as f:
        tasks = [json.loads(line) for line in f]
    
    print(f"✓ Loaded {len(tasks)} functions")
    
    # Initialize retriever ONCE (builds index, ~1.5s)
    print(f"\n🔧 Initializing hybrid retriever for {repo}...")
    print(f"   This will take ~1.5s (builds BM25 + embeddings index)")
    
    start_time = time.time()
    
    # Don't mask any function during indexing (we'll exclude per-query)
    retriever = HybridCodeRetriever(
        repo_path=str(repo_path),
        mask_function_id=None,  # No masking during indexing
        vllm_ports=vllm_ports
    )
    
    index_time = time.time() - start_time
    print(f"✓ Index built in {index_time:.2f}s")
    print(f"   Indexed {len(retriever.corpus)} chunks from context/ folder")
    
    # Generate prompts for all functions
    print(f"\n📝 Generating prompts with retrieval context...")
    prompts = []
    
    retrieval_start = time.time()
    
    for i, task in enumerate(tasks, 1):
        namespace = task["namespace"]
        signature = task["signature"]
        requirement = task["requirement"]
        function_id = task.get("function_id", "")
        completion_path = task.get("completion_path", "")
        
        # Build query from function info
        query_parts = [
            signature,
            requirement.get("Functionality", ""),
            requirement.get("Arguments", "")
        ]
        query = " ".join(query_parts)
        
        # Retrieve relevant context with smart filtering
        results = retriever.retrieve(
            query=query,
            top_k=top_k,
            stage1_candidates=stage1_candidates,
            max_chars_per_file=max_chars_per_result,
            exclude_target_file=False,  # Don't exclude - functions are already masked!
            file_type_weights=file_type_weights,
            exclude_test_files=exclude_test_files,
            min_code_ratio=min_code_ratio
        )
        
        # Build retrieved context section
        context_sections = []
        for idx, (file_path, content, score, breakdown) in enumerate(results, 1):
            rel_path = file_path.relative_to(repo_path / "context")
            context_sections.append(
                f"// ═══════════════════════════════════════════════════════════\n"
                f"// Context {idx}/{len(results)}: {rel_path}\n"
                f"// Relevance: {score:.3f} (BM25: {breakdown['bm25']:.1f}, Dense: {breakdown['dense']:.3f})\n"
                f"// ═══════════════════════════════════════════════════════════\n"
                f"{content}\n"
            )
        
        retrieved_context = "\n".join(context_sections) if context_sections else "// No relevant context found"
        
        # Build prompt with retrieval context (simple, single format)
        prompt = f"""You are an expert C++ programmer. Complete the following function implementation.

Function: {namespace}
Signature: {signature}

Requirements:
- Functionality: {requirement['Functionality']}
- Arguments: {requirement['Arguments']}

═══════════════════════════════════════════════════════════
RETRIEVED CONTEXT (Top {len(results)} relevant files)
═══════════════════════════════════════════════════════════

{retrieved_context}

═══════════════════════════════════════════════════════════
YOUR TASK
═══════════════════════════════════════════════════════════

Using the retrieved context above, implement the function body.

CRITICAL OUTPUT FORMAT:
- Provide ONLY the function body code (no signature, no braces)
- Start directly with the implementation
- Use std:: prefix for all standard library types/functions
- Do NOT include #include statements
- Do NOT include the function signature or wrapper

Example of CORRECT format:
```cpp
int count = 0;
for (int i = 0; i < n; i++) {{
    count++;
}}
return count;
```

Example of WRONG format (DO NOT do this):
```cpp
int MyClass::function_name(int n) {{  // ← NO signature!
    return n;
}}  // ← NO function braces!
```

Provide your implementation:
"""
        
        prompts.append({
            "namespace": namespace,
            "prompt": prompt,
            "metadata": task,
            "retrieval_info": {
                "top_k": len(results),
                "retrieved_files": [str(r[0].relative_to(repo_path / "context")) for r in results],
                "scores": [r[2] for r in results]
            }
        })
        
        # Progress indicator
        if i % 10 == 0 or i == len(tasks):
            elapsed = time.time() - retrieval_start
            avg_time = elapsed / i
            print(f"   Progress: {i}/{len(tasks)} ({i/len(tasks)*100:.0f}%) - {avg_time*1000:.1f}ms per function")
    
    retrieval_time = time.time() - retrieval_start
    
    # Save prompts
    output_file = output_dir / "prompts.jsonl"
    with open(output_file, "w") as f:
        for p in prompts:
            f.write(json.dumps(p) + "\n")
    
    # Summary
    total_time = time.time() - start_time
    print(f"\n{'='*70}")
    print(f"✅ SUCCESS")
    print(f"{'='*70}")
    print(f"Generated: {len(prompts)} prompts with retrieval context")
    print(f"Output: {output_file}")
    print(f"\nPerformance:")
    print(f"  Index build: {index_time:.2f}s (one-time cost)")
    print(f"  Retrieval: {retrieval_time:.2f}s ({retrieval_time/len(tasks)*1000:.1f}ms per function)")
    print(f"  Total: {total_time:.2f}s")
    print(f"  Speedup: {len(tasks)/total_time:.1f} functions/second")
    print(f"{'='*70}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate prompts with retrieval-augmented context"
    )
    parser.add_argument("--repo", required=True, help="Repository name (e.g., octree)")
    parser.add_argument(
        "--setting",
        default="baseline_retrieval",
        help="Setting name (e.g., baseline_retrieval, local_completion_retrieval)"
    )
    parser.add_argument(
        "--output_dir",
        type=Path,
        help="Output directory (default: results/{repo}/{setting})"
    )
    parser.add_argument(
        "--top_k",
        type=int,
        default=5,
        help="Number of retrieved contexts per function (default: 5)"
    )
    parser.add_argument(
        "--stage1_candidates",
        type=int,
        default=100,
        help="Number of BM25 candidates for diversity (default: 100)"
    )
    parser.add_argument(
        "--max_chars_per_result",
        type=int,
        default=5000,
        help="Max characters per retrieved file (default: 5000)"
    )
    parser.add_argument(
        "--exclude_test_files",
        action="store_true",
        help="Completely exclude test files from results"
    )
    parser.add_argument(
        "--min_code_ratio",
        type=float,
        default=0.3,
        help="Minimum code-to-comment ratio (0.0-1.0, default: 0.3)"
    )
    parser.add_argument(
        "--boost_cpp",
        type=float,
        default=1.5,
        help="Score multiplier for .cpp files (default: 1.5)"
    )
    parser.add_argument(
        "--boost_header",
        type=float,
        default=1.2,
        help="Score multiplier for .h files (default: 1.2)"
    )
    parser.add_argument(
        "--test_weight",
        type=float,
        default=0.5,
        help="Score multiplier for test files (default: 0.5)"
    )
    parser.add_argument(
        "--vllm_ports",
        nargs="+",
        type=int,
        default=[8000, 8001, 8002, 8003],
        help="vLLM server ports (default: 8000 8001 8002 8003)"
    )
    
    args = parser.parse_args()
    
    # Set default output directory
    if args.output_dir is None:
        args.output_dir = Path(f"results/retrieval/{args.repo}/{args.setting}")
    
    # Build file type weights from arguments
    file_type_weights = {
        'h': args.boost_header,
        'hpp': args.boost_header,
        'hxx': args.boost_header,
        'cpp': args.boost_cpp,
        'cc': args.boost_cpp,
        'cxx': args.boost_cpp,
        'test': args.test_weight,
    }
    
    generate_prompts_with_retrieval(
        repo=args.repo,
        setting=args.setting,
        output_dir=args.output_dir,
        top_k=args.top_k,
        max_chars_per_result=args.max_chars_per_result,
        vllm_ports=args.vllm_ports,
        file_type_weights=file_type_weights,
        exclude_test_files=args.exclude_test_files,
        min_code_ratio=args.min_code_ratio,
        stage1_candidates=args.stage1_candidates
    )


if __name__ == "__main__":
    main()
