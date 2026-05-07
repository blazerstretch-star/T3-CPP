#!/usr/bin/env python3
"""
Single-Pass Zero-Shot Evaluation
No tools, no iteration - just one prompt, one answer
"""

import os
import sys
import json
import argparse
import requests
import time
from pathlib import Path
from concurrent.futures import ProcessPoolExecutor, as_completed
import multiprocessing

# Add parent directory to path
sys.path.append(str(Path(__file__).parent))

from run_api_agent import (
    extract_code, 
    ast_quality_score,
    call_api_with_tools,
    log_error
)
from core.robust_injector import RobustDockerEvaluator
from core.trajectory_builder import TrajectoryBuilder


def build_zero_shot_prompt(task: dict) -> str:
    """Build a single-pass zero-shot prompt
    
    Args:
        task: Task metadata from metadata.jsonl
    
    Returns:
        Complete prompt string
    """
    namespace = task.get('namespace', '')
    signature = task.get('signature', '')
    requirement = task.get('requirement', {})
    functionality = requirement.get('Functionality', '')
    arguments = requirement.get('Arguments', '')
    
    return f"""

You are an expert C++ programmer. Complete the following function implementation.

Function: {namespace}
Signature: {signature}

Requirements:
- Functionality: {functionality}
- Arguments: {arguments}

⚠️⚠️⚠️ TOKEN LIMIT: 2000 TOKENS - WRITE CODE DIRECTLY, NO REASONING, NO EXPLANATIONS ⚠️⚠️⚠️

CRITICAL OUTPUT FORMAT:
- Provide ONLY the function body code (no signature, no braces)
- Start directly with the implementation
- Use std:: prefix for all standard library types/functions
- Do NOT include #include statements
- Do NOT include the function signature or wrapper
- End your response with a line containing only: // END_CODE

Example of CORRECT format:
```cpp
int count = 0;
for (int i = 0; i < n; i++) {{
    count++;
}}
return count;
// END_CODE
```

Example of WRONG format (DO NOT do this):
```cpp
int MyClass::function_name(int n) {{  // ← NO signature!
    return n;
}}  // ← NO function braces!
```

Provide your implementation and end with // END_CODE:
"""


def evaluate_single_pass(task: dict, code: str, repo_path: str, 
                        evaluator: RobustDockerEvaluator) -> dict:
    """Evaluate a single-pass completion
    
    Args:
        task: Task metadata
        code: Generated code
        repo_path: Path to repository
        evaluator: Docker evaluator instance
    
    Returns:
        Evaluation results dict
    """
    if not code:
        return {
            'compiles': False,
            'tests_pass': None,
            'test_summary': 'NO_CODE_EXTRACTED',
            'errors': ['No code extracted from model response']
        }
    
    try:
        repo_name = Path(repo_path).name
        source_file = task.get('completion_path', '')
        function_id = task.get('function_id', '')
        
        if not source_file or not function_id:
            return {
                'compiles': False,
                'tests_pass': None,
                'test_summary': 'MISSING_METADATA',
                'errors': ['Missing completion_path or function_id in metadata']
            }
        
        # Test compilation and execution
        success, payload = evaluator.inject_and_test_docker(
            repo_name=repo_name,
            source_file=source_file,
            completion=code,
            function_id=function_id,
            filter_test_output=False
        )
        
        # Parse results
        if isinstance(payload, list):
            return {
                'compiles': success,
                'tests_pass': None,
                'test_summary': 'COMPILATION_FAILED',
                'errors': payload
            }
        
        filtered = payload.get('filtered', []) if isinstance(payload, dict) else []
        full = payload.get('full_test_results') if isinstance(payload, dict) else None
        
        compiles = len(filtered) == 0
        tests_pass = None
        test_summary = None
        
        if isinstance(full, dict):
            rc = full.get('returncode')
            if rc is not None:
                tests_pass = (rc == 0)
                test_summary = 'PASSED' if tests_pass else 'FAILED'
        
        result = {
            'compiles': compiles,
            'tests_pass': tests_pass,
            'test_summary': test_summary,
            'errors': filtered[:10]  # Limit errors for storage
        }
        
        if full is not None:
            result['full_test_results'] = full
        
        return result
    
    except Exception as e:
        return {
            'compiles': False,
            'tests_pass': None,
            'test_summary': 'EVALUATION_ERROR',
            'errors': [f"Evaluation failed: {str(e)}"]
        }


def process_repository_single_pass(repo: str, args, api_key: str = None, api_key_idx: int = None):
    """Process repository in single-pass mode
    
    Args:
        repo: Repository name
        args: Command-line arguments
        api_key: API key assigned to this repository
        api_key_idx: Index of the API key (for display)
    """
    
    # Override API key if provided (for parallel execution)
    if api_key:
        args.api_key = api_key
    
    print(f"\n{'='*80}")
    print(f"📦 REPOSITORY: {repo} (Single-Pass Mode)")
    if api_key_idx:
        print(f"🔑 Using API Key #{api_key_idx}")
    print(f"{'='*80}\n")
    
    # Load metadata
    metadata_path = f"benchmarks/{repo}/metadata.jsonl"
    if not Path(metadata_path).exists():
        print(f"❌ Metadata not found: {metadata_path}")
        return
    
    with open(metadata_path) as f:
        tasks = [json.loads(line) for line in f]
    
    # Load pre-generated prompts if available (for retrieval-augmented settings)
    pregenerated_prompts = {}
    if args.use_pregenerated_prompts:
        if args.prompts_path:
            prompts_file = Path(args.prompts_path.format(repo=repo))
        else:
            prompts_file = Path(f"results/retrieval/{repo}/{args.setting}/prompts.jsonl")
        
        if prompts_file.exists():
            print(f"📥 Loading pre-generated prompts from: {prompts_file.absolute()}")
            with open(prompts_file) as f:
                for line in f:
                    try:
                        prompt_data = json.loads(line)
                        namespace = prompt_data.get('namespace')
                        if namespace:
                            pregenerated_prompts[namespace] = prompt_data['prompt']
                    except Exception:
                        pass
            print(f"✅ Successfully loaded {len(pregenerated_prompts)} pre-generated prompts from: {prompts_file.absolute()}")
        else:
            print(f"⚠️  No pre-generated prompts found at: {prompts_file.absolute()}")
            print(f"   Run: python scripts/generate_prompts_with_retrieval.py --repo {repo} --setting {args.setting}")
            if args.require_pregenerated:
                print(f"❌ Exiting (--require_pregenerated is set)")
                return
            print(f"   Continuing with standard prompt generation...")
    
    # Filter tasks if specific functions requested
    if args.functions:
        function_ids = set(args.functions)
        tasks = [t for t in tasks if t.get('function_id') in function_ids or t.get('task_id') in function_ids]
        if not tasks:
            print(f"⚠️  No matching functions found in {repo}")
            return
        print(f"🎯 Processing {len(tasks)} specific functions: {', '.join(function_ids)}")
    
    # Initialize evaluator
    evaluator = RobustDockerEvaluator(Path(f"benchmarks"))
    
    # Note: output_dir will be set per-function if using function_settings
    # Otherwise use the global setting
    if not hasattr(args, 'function_settings') or args.function_settings is None:
        output_setting = args.output_setting if args.output_setting else args.setting
        output_dir = Path(f"results/{args.model}/{repo}/{output_setting}")
        output_dir.mkdir(parents=True, exist_ok=True)
        print(f"💾 Output directory: {output_dir.absolute()}")
        error_log_file = output_dir / "single_pass_errors.log"
    else:
        output_dir = None
        error_log_file = None
    
    # Process tasks
    completions = []
    tasks_to_process = tasks[:args.limit] if args.limit else tasks
    
    for i, task in enumerate(tasks_to_process):
        namespace = task['namespace']
        task_id = task.get('function_id', namespace)
        
        print(f"\n{'='*60}")
        print(f"📝 [{i+1}/{len(tasks_to_process)}] {namespace}")
        print(f"{'='*60}")
        
        # Determine output directory and setting for this task
        if hasattr(args, 'function_settings') and args.function_settings and task_id in args.function_settings:
            task_setting = args.function_settings[task_id]
            task_output_dir = Path(f"results/{args.model}/{repo}/{task_setting}")
            task_output_dir.mkdir(parents=True, exist_ok=True)
            task_error_log = task_output_dir / "single_pass_errors.log"
            print(f"  🎯 Using setting: {task_setting}")
        else:
            task_setting = args.setting
            task_output_dir = output_dir
            task_error_log = error_log_file
        
        # Generate multiple completions if beam_width > 1
        beam_results = []
        
        for beam_id in range(1, args.beam_width + 1):
            if args.beam_width > 1:
                print(f"\n🔹 Beam {beam_id}/{args.beam_width}")
            
            # Check if trajectory already exists (resume mode)
            traj_path = TrajectoryBuilder.create_output_path(
                base_dir=Path("results"),
                repo=repo,
                task_id=task_id,
                model=args.model,
                beam_id=beam_id,
                setting=task_setting
            )
            
            skip_trajectory = False
            if args.resume and traj_path.exists():
                try:
                    with open(traj_path, 'r') as f:
                        trj_data = json.load(f)
                    
                    # Only redo trajectories with API errors or invalid budget mode
                    should_redo = False
                    redo_reason = []
                    
                    # Filter 0: Budget mode must be single_pass
                    budget = trj_data.get('budget', {})
                    budget_mode = budget.get('mode', '')
                    if budget_mode != 'single_pass':
                        should_redo = True
                        redo_reason.append(f"invalid_budget_mode_{budget_mode}")
                    
                    # Filter 1: API errors in trajectory steps
                    steps = trj_data.get('trajectory', [])
                    for step_idx, step in enumerate(steps):
                        api_status = step.get('api_status', 'ok')
                        if api_status != 'ok':
                            should_redo = True
                            redo_reason.append(f"api_error_{api_status}_step{step_idx}")
                            break
                        
                        # Check for API error messages in step
                        error_msg = step.get('error', '')
                        if error_msg and ('api' in error_msg.lower() or 'timeout' in error_msg.lower()):
                            should_redo = True
                            redo_reason.append(f"api_error_message_step{step_idx}")
                            break
                    
                    # Filter 2: Timeout detection (API-related)
                    if not should_redo:
                        metrics = trj_data.get('metrics', {})
                        stop_reason = metrics.get('stop_reason', '')
                        
                        # Only redo if timeout (API-related issue)
                        if stop_reason == 'timeout':
                            should_redo = True
                            redo_reason.append("timeout")
                    
                    # REMOVED Filter 3: Do NOT redo missing test results
                    # Missing test results are model behavior issues, not API errors
                    # Examples: no_code_extracted, compilation_failed, logic_error
                    # These should be skipped in resume mode
                    
                    if should_redo:
                        reason_str = ", ".join(redo_reason)
                        print(f"⚠️  {reason_str} detected: redoing {namespace} beam {beam_id}")
                        skip_trajectory = False
                    else:
                        skip_trajectory = True
                        
                except Exception as e:
                    print(f"⚠️  Error reading trajectory for resume check: {e}")
                    skip_trajectory = False
            
            if skip_trajectory:
                print(f"⏭️  Skipped {namespace} beam {beam_id}: trajectory already complete")
                continue
            
            # Build prompt (use pre-generated if available, otherwise build on-the-fly)
            if namespace in pregenerated_prompts:
                prompt = pregenerated_prompts[namespace]
                # Inject token budget warning at the start of pre-generated prompts
                if "⚠️⚠️⚠️" not in prompt:
                    prompt = f"⚠️⚠️⚠️ CRITICAL: YOU HAVE A {args.max_tokens} TOKEN LIMIT - DO NOT WASTE TOKENS ON REASONING OR EXPLANATIONS ⚠️⚠️⚠️\n\n" + prompt
                print(f"  📥 Using pre-generated prompt (loaded from file, token warning injected)")
            else:
                prompt = build_zero_shot_prompt(task)
                if args.use_pregenerated_prompts:
                    print(f"  ⚠️  No pre-generated prompt found for '{namespace}', generating standard prompt")
            
            # Log prompt
            log_file = task_output_dir / f"{namespace.replace('::', '_')}_beam_{beam_id}_log.txt"
            with open(log_file, 'w') as f:
                f.write("="*80 + "\n")
                f.write(f"SINGLE-PASS ZERO-SHOT EVALUATION\n")
                f.write(f"TASK: {namespace}\n")
                f.write(f"MODEL: {args.model}\n")
                f.write(f"SETTING: {task_setting}\n")
                f.write(f"BEAM: {beam_id}/{args.beam_width}\n")
                f.write(f"TIMESTAMP: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write("="*80 + "\n\n")
                f.write("PROMPT:\n")
                f.write("-"*80 + "\n")
                f.write(prompt)
                f.write("\n" + "-"*80 + "\n\n")
            
            # Call API (single pass, no tools)
            print(f"  ⏳ Calling API (single-pass, temperature={args.temperature})...")
            api_start = time.time()
            
            response = call_api_with_tools(
                conversation=[
                    {"role": "system", "content": f"You are an expert C++ programmer. CRITICAL: You have a {args.max_tokens} token limit. Write code directly without reasoning or explanations."},
                    {"role": "user", "content": prompt}
                ],
                tools=None,  # No tools in single-pass mode
                model=args.model,
                api_key=args.api_key,
                cooldown=args.api_cooldown,
                max_tokens=args.max_tokens,  # Use command-line argument
                error_log_file=str(task_error_log),
                temperature=args.temperature
            )
            
            api_duration = time.time() - api_start
            
            # Determine API status - only mark as error if genuine API error with status code
            api_status = "ok"
            api_error = "na"
            
            # Check for genuine API errors (status codes, connection errors, timeouts)
            if 'status_code' in response:
                status_code = response['status_code']
                if status_code != 200 and status_code is not None:
                    # Genuine API error with status code
                    api_status = "error"
                    error_msg = response.get('error_message', '') or response.get('response_text', '')
                    api_error = f"API Error {status_code}: {error_msg[:500]}"
            elif 'error_message' in response and response.get('error_message'):
                # Connection errors, timeouts, etc.
                api_status = "error"
                api_error = f"API Error: {response['error_message'][:500]}"
            
            # Note: Empty content or no code extracted is NOT an API error
            # It's a model behavior issue, not an infrastructure/API issue
            
            # Extract code
            code = extract_code(response.get('content', '') or '')
            
            # Log response
            with open(log_file, 'a') as f:
                f.write(f"API RESPONSE (duration: {api_duration:.2f}s):\n")
                f.write("-"*80 + "\n")
                f.write(json.dumps(response, indent=2, default=str))
                f.write("\n" + "-"*80 + "\n\n")
                f.write(f"EXTRACTED CODE ({len(code)} chars):\n")
                f.write("-"*80 + "\n")
                f.write(code if code else "[No code extracted]")
                f.write("\n" + "-"*80 + "\n\n")
            
            print(f"  📦 Code extracted: {len(code)} chars")
            
            # Evaluate
            print(f"  🔍 Evaluating...")
            eval_result = evaluate_single_pass(
                task, 
                code, 
                f"benchmarks/{repo}",
                evaluator
            )
            
            # Log evaluation
            with open(log_file, 'a') as f:
                f.write("EVALUATION RESULT:\n")
                f.write("-"*80 + "\n")
                f.write(json.dumps(eval_result, indent=2, default=str))
                f.write("\n" + "-"*80 + "\n\n")
            
            compiles = eval_result.get('compiles', False)
            tests_pass = eval_result.get('tests_pass', None)
            
            if compiles and tests_pass:
                print(f"  ✅ PASSED (compiles + tests pass)")
            elif compiles:
                print(f"  ⚠️  COMPILED but tests failed/unknown")
            else:
                errors = eval_result.get('errors', [])
                print(f"  ❌ COMPILATION FAILED ({len(errors)} errors)")
                if errors:
                    print(f"     First error: {errors[0][:100]}")
            
            # Build trajectory
            trajectory = TrajectoryBuilder(
                task_id=task_id,
                repo=repo,
                model=args.model,
                budget={'mode': 'single_pass'},
                beam_id=beam_id,
                temperature=args.temperature
            )
            
            # Single step trajectory
            usage = response.get('usage', {})
            trajectory.add_step(
                step_id=1,
                state={'mode': 'single_pass'},
                reasoning="Single-pass zero-shot completion (no reasoning chain)",
                action={'type': 'code_submission', 'code': code, 'code_length': len(code)},
                result=eval_result,
                api_status=api_status,
                api_error=api_error,
                api_duration=api_duration,
                tokens_used=usage.get('total_tokens', 0),
                token_breakdown={
                    'completion_tokens': usage.get('completion_tokens', 0),
                    'prompt_tokens': usage.get('prompt_tokens', 0),
                    'total_tokens': usage.get('total_tokens', 0)
                }
            )
            
            # Set final submission
            trajectory.set_final_submission(
                code=code,
                compiled=compiles,
                compile_error=None if compiles else '\n'.join(eval_result.get('errors', [])),
                failure_category='success' if (compiles and tests_pass) else 
                                'compilation_error' if not compiles else 'logic_error',
                compile_attempts=1,
                last_compile_errors=eval_result.get('errors', []),
                ast_score=ast_quality_score(code) if code else 0.0
            )
            
            trajectory.set_metrics(
                total_steps=1,
                total_tool_calls=0,
                tokens_used=usage.get('total_tokens', 0),
                wall_clock_sec=api_duration,
                stop_reason='single_pass_complete'
            )
            
            # Add test results
            if eval_result.get('full_test_results'):
                trajectory.test_results = eval_result['full_test_results']
                trajectory.passed_tests = tests_pass or False
            
            # Skip code similarity computation (disabled)
            # trajectory.code_similarity = None
            
            # Save trajectory
            trajectory.save(traj_path)
            print(f"  💾 Saved trajectory: {traj_path.name}")
            
            beam_results.append({
                'code': code,
                'eval': eval_result,
                'tokens': usage.get('total_tokens', 0)
            })
        
        # Add to completions
        if beam_results:
            completions.append({
                "namespace": namespace,
                "completions": [r['code'] for r in beam_results],
                "single_pass_metrics": {
                    'total_tokens': sum(r['tokens'] for r in beam_results),
                    'avg_tokens': sum(r['tokens'] for r in beam_results) / len(beam_results),
                    'beam_count': len(beam_results)
                }
            })
    
    # Save completions (only if we have a single output_dir)
    if output_dir is not None:
        output_file = output_dir / "completions.jsonl"
        existing_completions = {}
        
        if output_file.exists():
            with open(output_file, 'r') as f:
                for line in f:
                    try:
                        comp = json.loads(line)
                        if 'namespace' in comp:
                            existing_completions[comp['namespace']] = comp
                    except Exception:
                        pass
        
        # Merge
        for comp in completions:
            existing_completions[comp['namespace']] = comp
        
        # Write
        with open(output_file, 'w') as f:
            for comp in existing_completions.values():
                f.write(json.dumps(comp) + '\n')
        
        print(f"\n✅ Saved {len(existing_completions)} completions to {output_file}")
    else:
        print(f"\n✅ Completed {len(completions)} functions (trajectories saved to individual settings)")


def main():
    parser = argparse.ArgumentParser(description="Single-pass zero-shot evaluation")
    
    # Required
    parser.add_argument('--model', default='qwen/qwen3.5-397b-a17b', help='Model name')
    parser.add_argument('--repo', action='append', help='Repository name(s)')
    parser.add_argument('--repos', help='Comma-separated list of repositories, or "all"')
    parser.add_argument('--setting', default='baseline',
                       help='Evaluation setting name (used for output directory)')
    
    # API
    parser.add_argument('--api_key', default=os.environ.get('INFERENCE_API_KEY', ''),
                       help='Inference provider API key')
    parser.add_argument('--api_keys', type=str, default=None,
                       help='Comma-separated list of API keys for round-robin across repos')
    parser.add_argument('--api_keys_file', type=str, default=None,
                       help='File containing API keys (one per line) for round-robin')
    parser.add_argument('--api_cooldown', type=float, default=2.0,
                       help='Seconds between API calls')
    
    # Generation
    parser.add_argument('--max_tokens', type=int, default=2000,
                       help='Max tokens per completion (increased for thinking models like stepfun-ai)')
    parser.add_argument('--temperature', type=float, default=0.0,
                       help='Sampling temperature (0.0=greedy)')
    parser.add_argument('--beam_width', type=int, default=1,
                       help='Number of completions per task')
    
    # Control
    parser.add_argument('--limit', type=int, default=None,
                       help='Limit number of tasks')
    parser.add_argument('--resume', action='store_true', default=False,
                       help='Skip tasks with existing complete trajectories (only redo API errors)')
    parser.add_argument('--functions', nargs='+', default=None,
                       help='Specific function IDs to process (e.g., func032 leveldb_func023)')
    parser.add_argument('--functions_file', type=str, default=None,
                       help='JSON file with function list (from api_errors_main_modes_exact_settings.json)')
    parser.add_argument('--use_pregenerated_prompts', action='store_true', default=False,
                       help='Use pre-generated prompts from results/retrieval/{repo}/{setting}/prompts.jsonl')
    parser.add_argument('--require_pregenerated', action='store_true', default=False,
                       help='Exit if pre-generated prompts not found (use with --use_pregenerated_prompts)')
    parser.add_argument('--output_setting', type=str, default=None,
                       help='Custom setting name for output directory (default: same as --setting)')
    parser.add_argument('--prompts_path', type=str, default=None,
                       help='Custom path to prompts.jsonl file (overrides default retrieval path)')
    
    args = parser.parse_args()
    
    # Load functions from file if provided
    if args.functions_file:
        try:
            with open(args.functions_file, 'r') as f:
                data = json.load(f)
            
            # Extract functions and settings for this model
            model_functions = {}  # repo -> [task_ids]
            model_settings = {}   # repo -> {task_id -> setting}
            
            for model_data in data.get('api_errors_main_modes_only', []):
                if model_data['model'] == args.model:
                    for error in model_data['errors']:
                        repo = error['repo']
                        task_id = error['task_id']
                        setting = error['setting']
                        
                        if repo not in model_functions:
                            model_functions[repo] = []
                            model_settings[repo] = {}
                        
                        model_functions[repo].append(task_id)
                        model_settings[repo][task_id] = setting
            
            if model_functions:
                print(f"📋 Loaded {sum(len(v) for v in model_functions.values())} functions from {args.functions_file}")
                args.functions_by_repo = model_functions
                args.settings_by_repo = model_settings
            else:
                print(f"⚠️  No functions found for model {args.model} in {args.functions_file}")
                args.functions_by_repo = {}
                args.settings_by_repo = {}
        except Exception as e:
            print(f"❌ Error loading functions file: {e}")
            sys.exit(1)
    else:
        args.functions_by_repo = None
        args.settings_by_repo = None
    
    # Parse repositories
    repos = []
    if args.repo:
        if 'all' in args.repo:
            repos = get_all_repos()
        else:
            repos.extend(args.repo)
    if args.repos:
        if args.repos.lower() == 'all':
            repos = get_all_repos()
        else:
            repos.extend([r.strip() for r in args.repos.split(',')])
    
    if not repos:
        print("❌ No repositories specified. Use --repo REPO or --repos all")
        sys.exit(1)
    
    repos = list(set(repos))
    
    # Parse API keys for round-robin
    api_keys = []
    if args.api_keys:
        # Comma-separated API keys
        api_keys = [k.strip() for k in args.api_keys.split(',') if k.strip()]
    elif args.api_keys_file:
        # Load from file (one per line)
        try:
            with open(args.api_keys_file, 'r') as f:
                api_keys = [line.strip() for line in f if line.strip() and not line.strip().startswith('#')]
        except Exception as e:
            print(f"❌ Error reading API keys file: {e}")
            sys.exit(1)
    elif args.api_key:
        # Single API key
        api_keys = [args.api_key]
    
    # Check API keys
    if not api_keys:
        print("❌ No API key provided")
        print("Usage options:")
        print("  1. Single key: export INFERENCE_API_KEY='<api-key>...'")
        print("  2. Multiple keys: --api_keys 'key1,key2,key3'")
        print("  3. Keys file: --api_keys_file api_keys.txt")
        sys.exit(1)
    
    print(f"🔑 Loaded {len(api_keys)} API key(s) for round-robin distribution")
    if len(api_keys) > 1:
        print(f"   Each repository will use a different API key (round-robin)")
    
    # Round-robin API key assignment
    repo_api_mapping = {}
    for repo_idx, repo in enumerate(repos):
        api_key_idx = repo_idx % len(api_keys)
        repo_api_mapping[repo] = {
            'api_key': api_keys[api_key_idx],
            'api_key_idx': api_key_idx + 1
        }
    
    # Display API key distribution
    if len(api_keys) > 1:
        print(f"\n📊 API Key Distribution (Round-Robin):")
        print(f"{'='*80}")
        for repo in repos:
            mapping = repo_api_mapping[repo]
            key_preview = mapping['api_key'][:20] + '...' if len(mapping['api_key']) > 20 else mapping['api_key']
            print(f"  {repo:30s} → API Key #{mapping['api_key_idx']} ({key_preview})")
        print(f"{'='*80}\n")
    
    print(f"\n{'='*80}")
    print(f"🚀 SINGLE-PASS ZERO-SHOT EVALUATION (PARALLEL MODE)")
    print(f"{'='*80}")
    print(f"Model: {args.model}")
    print(f"Setting: {args.setting}")
    print(f"Temperature: {args.temperature}")
    print(f"Beam width: {args.beam_width}")
    print(f"Repositories: {', '.join(repos)}")
    print(f"API Keys: {len(api_keys)}")
    print(f"Parallel Workers: {min(len(repos), len(api_keys))}")
    print(f"{'='*80}\n")
    
    # Prepare repository tasks with their assigned API keys
    repo_tasks = []
    for repo_idx, repo in enumerate(repos):
        # Get assigned API key for this repo
        assigned_api_key = repo_api_mapping[repo]['api_key']
        assigned_api_idx = repo_api_mapping[repo]['api_key_idx']
        
        # Create a copy of args for this repo
        import copy
        repo_args = copy.deepcopy(args)
        repo_args.api_key = assigned_api_key
        
        # Set functions for this repo if using functions_file
        if args.functions_by_repo is not None:
            repo_args.functions = args.functions_by_repo.get(repo, [])
            if not repo_args.functions:
                print(f"⏭️  Skipping {repo}: no functions to re-evaluate")
                continue
            
            # Get settings for this repo's functions
            repo_args.function_settings = args.settings_by_repo.get(repo, {})
        else:
            repo_args.function_settings = None
        
        repo_tasks.append({
            'repo': repo,
            'args': repo_args,
            'api_key': assigned_api_key,
            'api_key_idx': assigned_api_idx
        })
    
    if not repo_tasks:
        print("❌ No repositories to process")
        return
    
    # Process repositories in parallel
    max_workers = min(len(repo_tasks), len(api_keys))
    print(f"🚀 Starting parallel processing with {max_workers} workers...\n")
    
    completed_count = 0
    failed_repos = []
    
    with ProcessPoolExecutor(max_workers=max_workers) as executor:
        # Submit all tasks
        future_to_repo = {
            executor.submit(
                process_repository_single_pass,
                task['repo'],
                task['args'],
                task['api_key'],
                task['api_key_idx']
            ): task['repo']
            for task in repo_tasks
        }
        
        # Process completed tasks
        for future in as_completed(future_to_repo):
            repo = future_to_repo[future]
            try:
                future.result()
                completed_count += 1
                print(f"\n✅ [{completed_count}/{len(repo_tasks)}] Completed: {repo}")
            except Exception as e:
                failed_repos.append(repo)
                print(f"\n❌ [{completed_count + len(failed_repos)}/{len(repo_tasks)}] Failed: {repo}")
                print(f"   Error: {e}")
                import traceback
                traceback.print_exc()
    
    # Summary
    print(f"\n{'='*80}")
    print(f"📊 PARALLEL EXECUTION SUMMARY")
    print(f"{'='*80}")
    print(f"✅ Completed: {completed_count}/{len(repo_tasks)}")
    if failed_repos:
        print(f"❌ Failed: {len(failed_repos)}/{len(repo_tasks)}")
        print(f"   Failed repos: {', '.join(failed_repos)}")
    print(f"{'='*80}\n")


def get_all_repos():
    """Get all available repositories"""
    benchmarks_dir = Path("benchmarks")
    if not benchmarks_dir.exists():
        return []
    repos = [d.name for d in benchmarks_dir.iterdir() 
             if d.is_dir() and (d / "metadata.jsonl").exists()]
    return sorted(repos)


if __name__ == "__main__":
    main()
