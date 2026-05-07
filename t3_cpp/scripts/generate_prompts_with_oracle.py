#!/usr/bin/env python3
"""
Generate prompts with oracle context for Condition C evaluation.

Oracle context = exact files that successful agents read during trajectories.
This represents the ceiling of perfect retrieval without iteration.
"""

import sys
import json
import argparse
import re
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))


def mask_function_in_content(content: str, function_id: str) -> str:
    """Mask ALL function implementations to prevent leakage
    
    Uses the same pattern as agentic_tools.py and hybrid_retriever_vllm.py:
    Masks ALL functions marked with FUNCTION_ID comments
    // FUNCTION_ID: <id> - START
    ...
    // FUNCTION_ID: <id> - END
    """
    # Mask ALL functions marked with FUNCTION_ID comments (same as retrieval)
    pattern = r'// FUNCTION_ID: ([^\n]+) - START.*?// FUNCTION_ID: \1 - END'
    replacement = r'// FUNCTION_ID: \1 - START\n  // [MASKED FOR NO LEAKAGE]\n  // FUNCTION_ID: \1 - END'
    
    masked_content = re.sub(pattern, replacement, content, flags=re.DOTALL)
    
    return masked_content


def generate_prompts_with_oracle(
    repo: str,
    setting: str,
    output_dir: Path,
    oracle_contexts_path: Path,
    max_chars_per_file: int = 5000
):
    """Generate oracle-context prompts for all functions in a repository.
    
    Args:
        repo: Repository name (e.g., "octree")
        setting: Setting name (e.g., "baseline_oracle")
        output_dir: Output directory for prompts.jsonl
        oracle_contexts_path: Path to oracle_contexts_331.json
        max_chars_per_file: Max characters per file
    """
    
    # Setup paths
    repo_path = Path("benchmarks") / repo
    metadata_path = repo_path / "metadata.jsonl"
    context_path = repo_path / "context"
    output_dir.mkdir(parents=True, exist_ok=True)
    
    if not metadata_path.exists():
        print(f"❌ Metadata not found: {metadata_path}")
        sys.exit(1)
    
    if not context_path.exists():
        print(f"❌ Context folder not found: {context_path}")
        sys.exit(1)
    
    if not oracle_contexts_path.exists():
        print(f"❌ Oracle contexts not found: {oracle_contexts_path}")
        sys.exit(1)
    
    # Load metadata
    print(f"📖 Loading metadata from {metadata_path}...")
    with open(metadata_path) as f:
        tasks = [json.loads(line) for line in f]
    
    print(f"✓ Loaded {len(tasks)} functions")
    
    # Load oracle contexts
    print(f"📖 Loading oracle contexts from {oracle_contexts_path}...")
    with open(oracle_contexts_path) as f:
        oracle_data = json.load(f)
    
    oracle_contexts = oracle_data["oracle_contexts"]
    print(f"✓ Loaded {len(oracle_contexts)} oracle contexts")
    
    # Generate prompts
    print(f"\n📝 Generating prompts with oracle context...")
    prompts = []
    functions_with_oracle = 0
    functions_without_oracle = 0
    
    for i, task in enumerate(tasks, 1):
        namespace = task["namespace"]
        signature = task["signature"]
        requirement = task["requirement"]
        function_id = task.get("function_id", "")
        
        # Use function_id as the key (e.g., "octree_func001")
        func_name = function_id
        
        # Check if we have oracle context for this function
        if func_name not in oracle_contexts:
            print(f"   ⚠️  No oracle context for {func_name}, skipping...")
            functions_without_oracle += 1
            continue
        
        oracle_info = oracle_contexts[func_name]
        oracle_files = oracle_info["files"]
        oracle_repo = oracle_info["repo"]
        
        # Verify repo matches
        if oracle_repo != repo:
            print(f"   ⚠️  Repo mismatch for {func_name}: expected {repo}, got {oracle_repo}")
            functions_without_oracle += 1
            continue
        
        functions_with_oracle += 1
        
        # Read oracle files from context/ folder
        context_sections = []
        files_read = 0
        files_not_found = 0
        
        for idx, file_path_str in enumerate(oracle_files, 1):
            # Oracle files are relative to repo root, need to check in context/
            file_path = context_path / file_path_str
            
            if not file_path.exists():
                # Try without context/ prefix (some oracle files might already be relative to context/)
                file_path = context_path / Path(file_path_str).name
                
                if not file_path.exists():
                    files_not_found += 1
                    continue
            
            try:
                with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                
                # Mask ALL functions if present
                content = mask_function_in_content(content, function_id)
                
                # Truncate if too long
                if len(content) > max_chars_per_file:
                    content = content[:max_chars_per_file] + "\n\n// ... (truncated)"
                
                # Format like retrieval context
                rel_path = file_path.relative_to(context_path)
                context_sections.append(
                    f"// ═══════════════════════════════════════════════════════════\n"
                    f"// Oracle Context {idx}/{len(oracle_files)}: {rel_path}\n"
                    f"// Source: {oracle_info['model']}\n"
                    f"// ═══════════════════════════════════════════════════════════\n"
                    f"{content}\n"
                )
                
                files_read += 1
            
            except Exception as e:
                print(f"   ⚠️  Error reading {file_path}: {e}")
                files_not_found += 1
        
        if files_not_found > 0:
            print(f"   ⚠️  {func_name}: {files_not_found}/{len(oracle_files)} files not found in context/")
        
        oracle_context = "\n".join(context_sections) if context_sections else "// No oracle context available"
        
        # Build prompt (same format as retrieval)
        prompt = f"""You are an expert C++ programmer. Complete the following function implementation.

Function: {namespace}
Signature: {signature}

Requirements:
- Functionality: {requirement['Functionality']}
- Arguments: {requirement['Arguments']}

═══════════════════════════════════════════════════════════
ORACLE CONTEXT (Exact files used by successful agent)
═══════════════════════════════════════════════════════════

{oracle_context}

═══════════════════════════════════════════════════════════
YOUR TASK
═══════════════════════════════════════════════════════════

Using the oracle context above, implement the function body.

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
            "oracle_info": {
                "num_files": len(oracle_files),
                "files_read": files_read,
                "files_not_found": files_not_found,
                "oracle_files": oracle_files,
                "source_model": oracle_info["model"],
                "trajectory_path": oracle_info.get("trajectory_path", ""),
                "passed_tests": oracle_info.get("passed_tests", True),
                "note": oracle_info.get("note", "")
            }
        })
        
        # Progress indicator
        if i % 10 == 0 or i == len(tasks):
            print(f"   Progress: {functions_with_oracle}/{len(tasks)} functions with oracle context")
    
    # Save prompts
    output_file = output_dir / "prompts.jsonl"
    with open(output_file, "w") as f:
        for p in prompts:
            f.write(json.dumps(p) + "\n")
    
    # Summary
    print(f"\n{'='*70}")
    print(f"✅ SUCCESS")
    print(f"{'='*70}")
    print(f"Generated: {len(prompts)} prompts with oracle context")
    print(f"Functions with oracle: {functions_with_oracle}")
    print(f"Functions without oracle: {functions_without_oracle}")
    print(f"Coverage: {functions_with_oracle}/{len(tasks)} ({functions_with_oracle/len(tasks)*100:.1f}%)")
    print(f"Output: {output_file}")
    print(f"{'='*70}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate prompts with oracle context (Condition C)"
    )
    parser.add_argument("--repo", required=True, help="Repository name (e.g., octree)")
    parser.add_argument(
        "--setting",
        default="baseline_oracle",
        help="Setting name (default: baseline_oracle)"
    )
    parser.add_argument(
        "--output_dir",
        type=Path,
        help="Output directory (default: results/oracle/{repo}/{setting})"
    )
    parser.add_argument(
        "--oracle_contexts",
        type=Path,
        default=Path("trajectories_331_filtered/oracle_contexts_331.json"),
        help="Path to oracle contexts file (default: trajectories_331_filtered/oracle_contexts_331.json)"
    )
    parser.add_argument(
        "--max_chars_per_file",
        type=int,
        default=10000,
        help="Max characters per file (default: 10000)"
    )
    
    args = parser.parse_args()
    
    # Set default output directory
    if args.output_dir is None:
        args.output_dir = Path(f"results/oracle/{args.repo}/{args.setting}")
    
    generate_prompts_with_oracle(
        repo=args.repo,
        setting=args.setting,
        output_dir=args.output_dir,
        oracle_contexts_path=args.oracle_contexts,
        max_chars_per_file=args.max_chars_per_file
    )


if __name__ == "__main__":
    main()
