#!/usr/bin/env python3
"""Fixed Docker-based Pass@k evaluation for C++ repositories"""

import json
import argparse
import subprocess
import tempfile
import os
import shutil
import uuid
from pathlib import Path
from tqdm import tqdm
import numpy as np
from code_and_train_ds.t3_cpp.core.robust_injector import RobustDockerEvaluator

class FixedCppDockerEvaluator(RobustDockerEvaluator):
    def __init__(self, source_code_root):
        super().__init__(source_code_root)
    
    def inject_and_test_docker(self, repo_name, source_file, completion, function_id, namespace=None, save_dir=None, attempt_num=None, filter_test_output=False):
        """Inject code using complete functions as baseline. Returns (success, test_output)
        
        Args:
            filter_test_output: If True, hide test results (for LLM). If False, return full output (for PRM).
        """
        repo_path = self.source_code_root / repo_name
        
        # Create temp directory
        temp_id = str(uuid.uuid4())[:8]
        temp_base = Path("/mnt/disk1/sub/test/tmp/t3_cpp")
        temp_base.mkdir(exist_ok=True)
        temp_repo = temp_base / f"{repo_name}_{temp_id}"
        
        try:
            # Copy repository
            shutil.copytree(repo_path, temp_repo, dirs_exist_ok=True)
            
            # Use complete functions as baseline
            source_path = temp_repo / source_file
            complete_source = temp_repo / source_file.replace('.cpp', '_complete.cpp')
            
            if complete_source.exists():
                shutil.copy2(complete_source, source_path)
            
            # Inject with backup using function ID
            success, backup_path = self.injector.inject_with_backup(
                source_path, completion, function_id
            )
            
            if not success:
                return False, None
            
            # Save cleaned completion for transparency
            if save_dir and namespace and attempt_num is not None:
                attempt_dir = Path(save_dir) / f"attempt_{attempt_num}"
                cleaned_path = attempt_dir / "cleaned_completions"
                cleaned_path.mkdir(parents=True, exist_ok=True)
                func_name = namespace.split('::')[-1]
                cleaned_file = cleaned_path / f"{func_name}.txt"
                cleaned_completion = self.injector.clean_completion(completion)
                with open(cleaned_file, 'w') as f:
                    f.write(cleaned_completion)
            
            # Save injected file for transparency
            if save_dir and namespace and attempt_num is not None:
                attempt_dir = Path(save_dir) / f"attempt_{attempt_num}"
                save_path = attempt_dir / "injected_files"
                save_path.mkdir(parents=True, exist_ok=True)
                func_name = namespace.split('::')[-1]
                injected_file = save_path / f"{func_name}.cpp"
                shutil.copy2(source_path, injected_file)
            
            # Make run_tests.sh executable
            run_tests_path = temp_repo / "run_tests.sh"
            if run_tests_path.exists():
                os.chmod(run_tests_path, 0o755)
            
            # Test in Docker
            container_name = f"t3_cpp-{repo_name.lower()}"
            
            # Ensure container exists
            if not self._container_exists(container_name):
                if not self._build_container(repo_name, container_name):
                    return False
            
            # Extract test name from namespace
            func_name = namespace.split('::')[-1] if namespace else None
            test_name = f"test_{func_name}" if func_name else None
            
            # Run only the specific test for this function using docker cp
            if test_name:
                container_id = subprocess.run([
                    "docker", "create", "-w", "/workspace",
                    container_name,
                    "/bin/bash", "-c",
                    f"cmake . && make && ctest -R {test_name} --output-on-failure"
                ], capture_output=True, text=True).stdout.strip()
                
                subprocess.run(["docker", "cp", f"{temp_repo}/.", f"{container_id}:/workspace"], check=True)
                
                result = subprocess.run([
                    "docker", "start", "-a", container_id
                ], capture_output=True, timeout=600, text=True)
                
                # Save logs for verification
                if save_dir and namespace and attempt_num is not None:
                    attempt_dir = Path(save_dir) / f"attempt_{attempt_num}"
                    log_path = attempt_dir / "test_logs"
                    log_path.mkdir(parents=True, exist_ok=True)
                    func_name = namespace.split('::')[-1]
                    log_file = log_path / f"{func_name}.log"
                    with open(log_file, 'w') as f:
                        f.write(f"=== Test: {test_name} ===\n")
                        f.write(f"Exit code: {result.returncode}\n\n")
                        f.write("=== STDOUT ===\n")
                        f.write(result.stdout)
                        f.write("\n=== STDERR ===\n")
                        f.write(result.stderr)
                
                subprocess.run(["docker", "rm", container_id], capture_output=True)
            else:
                # Fallback to running all tests
                container_id = subprocess.run([
                    "docker", "create", container_name
                ], capture_output=True, text=True).stdout.strip()
                
                subprocess.run(["docker", "cp", f"{temp_repo}/.", f"{container_id}:/workspace"], check=True)
                
                result = subprocess.run([
                    "docker", "start", "-a", container_id
                ], capture_output=True, timeout=600, text=True)
                
                subprocess.run(["docker", "rm", container_id], capture_output=True)
            
            success = result.returncode == 0
            
            # Parse output based on filter_test_output flag
            if not success:
                if filter_test_output:
                    # For LLM: only compilation errors (truncated to ~500 tokens)
                    errors = self._parse_compilation_errors(result.stdout + result.stderr, for_llm=True)
                else:
                    # For PRM: full output including test failures (no truncation)
                    errors = self._parse_full_output(result.stdout + result.stderr)
                test_output = {
                    "stdout": result.stdout, 
                    "stderr": result.stderr, 
                    "returncode": result.returncode,
                    "errors": errors
                }
            else:
                test_output = {
                    "stdout": result.stdout, 
                    "stderr": result.stderr, 
                    "returncode": result.returncode,
                    "errors": []
                }
            
            # Cleanup temp directory after Docker finishes
            shutil.rmtree(temp_repo, ignore_errors=True)
            
            return success, test_output
            
        except Exception as e:
            print(f"Error: {e}")
            shutil.rmtree(temp_repo, ignore_errors=True)
            return False, {"error": str(e)}
    
    def _container_exists(self, container_name):
        """Check if Docker container exists"""
        result = subprocess.run(
            ["docker", "images", "-q", container_name],
            capture_output=True, text=True
        )
        return bool(result.stdout.strip())
    
    def _build_container(self, repo_name, container_name):
        """Build Docker container for repository"""
        repo_path = self.source_code_root / repo_name
        
        cmd = ["docker", "build", "-t", container_name, str(repo_path)]
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode != 0:
            print(f"Failed to build container for {repo_name}: {result.stderr}")
            return False
        return True

def compute_pass_at_k(n, c, k):
    """Compute Pass@k metric"""
    if n - c < k:
        return 1.0
    return 1.0 - np.prod(1.0 - k / np.arange(n - c + 1, n + 1))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output_file', required=True)
    parser.add_argument('--source_code_root', required=True)
    parser.add_argument('--data_file', required=True)
    parser.add_argument('--k', default='1,3,5')
    parser.add_argument('--setting', choices=['baseline', 'local_completion', 'local_infilling'], default='baseline')
    parser.add_argument('--update_trajectories', action='store_true', default=True,
                        help='Update trajectory files with test results')
    args = parser.parse_args()
    
    evaluator = FixedCppDockerEvaluator(args.source_code_root)
    
    # Load data
    with open(args.data_file) as f:
        metadata = {json.loads(line)['namespace']: json.loads(line) for line in f}
    
    with open(args.output_file) as f:
        completions = [json.loads(line) for line in f]
    
    # Group completions by namespace
    results = {}
    k_values = [int(k) for k in args.k.split(',')]
    base_dir = Path(args.output_file).parent
    
    # Find trajectory directory - correct path format
    model_safe = base_dir.parent.parent.name.replace('/', '_')
    traj_dir = Path('results') / model_safe / base_dir.parent.name / 'trajectories'
    
    with tqdm(total=len(completions), desc="Evaluating C++ Pass@k", unit="test", leave=False) as pbar:
        for completion in completions:
            namespace = completion['namespace']
            if namespace in metadata:
                meta = metadata[namespace]
                
                # Extract repository info
                completion_path = meta['completion_path']
                repo_name = completion_path.split('/')[0]
                source_file = '/'.join(completion_path.split('/')[1:])
                function_id = meta.get('function_id', namespace)
                
                # Handle both 'completion' and 'completions' formats
                completions_list = completion.get('completions', [completion.get('completion', '')])
                
                if namespace not in results:
                    results[namespace] = []
                
                # Test each completion ONCE and store results
                for idx, completion_text in enumerate(completions_list, 1):
                    save_dir = base_dir / f"pass@{max(k_values)}"
                    success, _test_output = evaluator.inject_and_test_docker(
                        repo_name, source_file, completion_text, meta['function_id'],
                        namespace=namespace, save_dir=save_dir, attempt_num=idx
                    )
                    
                    results[namespace].append(success)
                    
                    # Update trajectory file with test results
                    if args.update_trajectories and traj_dir.exists():
                        traj_file = traj_dir / f"{function_id}_beam_{idx}.json"
                        if traj_file.exists():
                            try:
                                with open(traj_file, 'r') as f:
                                    traj_data = json.load(f)
                                
                                # Update final submission with test results
                                if 'final_submission' in traj_data:
                                    traj_data['final_submission']['compiled'] = success
                                    traj_data['final_submission']['passed_tests'] = success
                                    if not success:
                                        traj_data['final_submission']['compile_error'] = 'Tests failed'
                                
                                with open(traj_file, 'w') as f:
                                    json.dump(traj_data, f, indent=2)
                            except Exception as e:
                                print(f"Warning: Could not update trajectory {traj_file}: {e}")
                    
                    pbar.set_postfix({"Current": f"{namespace.split('::')[-1]}", "Attempt": idx})
                    pbar.update(1 / len(completions_list))
    
    # Calculate Pass@k and save evaluation.json for each k
    for k in k_values:
        if results:
            pass_rates = []
            detailed_results = {}
            for namespace, successes in results.items():
                n = len(successes)
                c = sum(successes)
                # Store results with beam_id for clarity
                detailed_results[namespace] = [
                    {"beam_id": i+1, "passed": success} 
                    for i, success in enumerate(successes[:k])
                ]
                if n >= k:
                    pass_rates.append(compute_pass_at_k(n, c, k))
            
            if pass_rates:
                avg_pass_rate = np.mean(pass_rates)
                print(f"Pass@{k}: {avg_pass_rate:.3f}")
                
                # Save evaluation.json for this k
                eval_data = {
                    "model": base_dir.parent.parent.name,
                    "repo": base_dir.parent.name,
                    "setting": base_dir.name,
                    "metrics": {f"pass@{k}": avg_pass_rate},
                    "detailed_results": detailed_results
                }
                eval_file = base_dir / f"pass@{k}" / "evaluation.json"
                eval_file.parent.mkdir(parents=True, exist_ok=True)
                with open(eval_file, 'w') as f:
                    json.dump(eval_data, f, indent=2)

if __name__ == '__main__':
    main()