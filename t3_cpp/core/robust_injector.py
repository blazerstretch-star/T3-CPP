#!/usr/bin/env python3
"""Robust brace-aware code injection system"""

import re
import shutil
import subprocess
from pathlib import Path

class RobustCodeInjector:
    def __init__(self, start_marker="// EVAL-START:", end_marker="// EVAL-END:"):
        self.start_marker = start_marker
        self.end_marker = end_marker
    
    def _normalize_indentation(self, code):
        """Convert tabs to spaces and normalize indentation"""
        if not code.strip():
            return code
        
        lines = code.split('\n')
        normalized_lines = []
        
        for line in lines:
            if not line.strip():
                normalized_lines.append('')
                continue
            
            # Convert tabs to 4 spaces
            expanded = line.expandtabs(4)
            normalized_lines.append(expanded)
        
        return '\n'.join(normalized_lines)
    
    def _detect_base_indentation(self, code):
        """Detect if code already has base indentation"""
        lines = [line for line in code.split('\n') if line.strip()]
        if not lines:
            return 0
        
        # Find minimum indentation of non-empty lines
        min_indent = float('inf')
        for line in lines:
            if line.strip():
                indent = len(line) - len(line.lstrip())
                min_indent = min(min_indent, indent)
        
        return min_indent if min_indent != float('inf') else 0
    
    def _smart_brace_count(self, code, start_pos=0):
        """Simple brace matching that ignores strings and comments"""
        brace_count = 1 if start_pos > 0 else 0
        i = start_pos
        in_string = False
        in_char = False
        string_char = None
        
        while i < len(code) and brace_count > 0:
            char = code[i]
            
            # Handle strings
            if not in_string and not in_char and char in ['"', "'"]:
                in_string = (char == '"')
                in_char = (char == "'")
                string_char = char
            elif (in_string and char == '"' and string_char == '"') or (in_char and char == "'" and string_char == "'"):
                if i > 0 and code[i-1] != '\\\\':
                    in_string = False
                    in_char = False
                    string_char = None
            
            # Skip line comments
            if not in_string and not in_char and i < len(code) - 1 and code[i:i+2] == '//':
                while i < len(code) and code[i] != '\n':
                    i += 1
                continue
            
            # Skip block comments
            if not in_string and not in_char and i < len(code) - 1 and code[i:i+2] == '/*':
                i += 2
                while i < len(code) - 1:
                    if code[i:i+2] == '*/':
                        i += 2
                        break
                    i += 1
                continue
            
            # Count braces only if not in string/comment
            if not in_string and not in_char:
                if char == '{':
                    brace_count += 1
                elif char == '}':
                    brace_count -= 1
                    if brace_count == 0:
                        return i
            
            i += 1
        
        return -1 if brace_count > 0 else i
    
    def find_function_by_markers(self, lines, function_id):
        """Find function boundaries using function ID markers - replace content between markers"""
        start_marker = f"FUNCTION_ID: {function_id} - START"
        end_marker = f"FUNCTION_ID: {function_id} - END"
        
        start_line = None
        end_line = None
        
        # Find markers
        for i, line in enumerate(lines):
            if start_marker in line.strip():
                start_line = i
            elif end_marker in line.strip():
                end_line = i
                break
        
        if start_line is None:
            raise ValueError(f"Start marker '{start_marker}' not found")
        if end_line is None:
            raise ValueError(f"End marker '{end_marker}' not found")
        
        # Return the lines between markers (exclusive of marker lines)
        return start_line, end_line
    

    def clean_completion(self, completion):
        """Clean and normalize completion"""
        # Remove markdown
        completion = re.sub(r'```(?:cpp|c\+\+)?\s*', '', completion)
        completion = re.sub(r'```\s*', '', completion)
        completion = completion.strip()
        
        if not completion:
            return completion
        
        # Normalize indentation (convert tabs to spaces)
        completion = self._normalize_indentation(completion)
        
        return completion
    
    def inject_with_backup(self, source_file, completion, function_id, backup_suffix='.backup'):
        """Inject code with automatic backup - marker-based only"""
        source_path = Path(source_file)
        backup_path = source_path.with_suffix(source_path.suffix + backup_suffix)
        
        try:
            # Create backup
            shutil.copy2(source_path, backup_path)
            
            # Read original file
            with open(source_path, 'r') as f:
                lines = f.readlines()
            
            # Use function ID marker-based approach only
            start_line, end_line = self.find_function_by_markers(lines, function_id)
            
            # Clean the completion
            clean_code = self.clean_completion(completion)
            
            # Detect if code already has base indentation
            base_indent_detected = self._detect_base_indentation(clean_code)
            
            # Prepare new function body - replace content between markers
            new_body = [lines[start_line]]  # Keep START marker line
            
            # Smart indentation handling
            base_indent = '  '  # 2 spaces to match existing code style
            for line in clean_code.split('\n'):
                if line.strip():  # If not empty
                    if base_indent_detected > 0:
                        # Remove detected base indentation, then add our base
                        if line.startswith(' ' * base_indent_detected):
                            line = line[base_indent_detected:]
                        new_body.append(base_indent + line + '\n')
                    else:
                        # No base indentation detected, add our base
                        new_body.append(base_indent + line + '\n')
                else:
                    new_body.append('\n')
            
            new_body.append(lines[end_line])  # Keep END marker line
            
            # Replace content between markers (inclusive of start, exclusive of end)
            lines[start_line:end_line+1] = new_body
            
            # Write modified file
            with open(source_path, 'w') as f:
                f.writelines(lines)
            
            return True, backup_path
            
        except Exception as e:
            # Restore from backup if injection failed
            if backup_path.exists():
                shutil.copy2(backup_path, source_path)
            raise e
    
    def restore_from_backup(self, source_file, backup_suffix='.backup'):
        """Restore original file from backup"""
        source_path = Path(source_file)
        backup_path = source_path.with_suffix(source_path.suffix + backup_suffix)
        
        if backup_path.exists():
            shutil.copy2(backup_path, source_path)
            backup_path.unlink()
            return True
        return False

class RobustDockerEvaluator:
    def __init__(self, source_code_root):
        self.source_code_root = Path(source_code_root)
        self.injector = RobustCodeInjector()
    
    def inject_and_test_docker(self, repo_name, source_file, completion, function_id, filter_test_output=True):
        """Inject code using function ID markers with Docker testing

        Args:
            filter_test_output: If True, hide test results (for LLM exploration).
                               If False, return full output (for PRM training).

        Returns:
            tuple: (success: bool, payload: dict)

        Payload contains:
            - 'filtered': list[str] or []  # truncated compilation errors for LLM
            - 'full_test_results': { 'stdout', 'stderr', 'returncode', 'errors' }
            - 'env': environment metadata (compiler version, docker image name)
        """
        repo_path = self.source_code_root / repo_name
        
        # Create temp directory in a Docker-friendly location
        import tempfile, uuid
        temp_id = str(uuid.uuid4())[:8]
        temp_base = Path("/mnt/disk1/sub/test/tmp/t3_cpp")
        temp_base.mkdir(exist_ok=True)
        temp_repo = temp_base / f"{repo_name}_{temp_id}"
        
        try:
            # Copy repository
            shutil.copytree(repo_path, temp_repo, dirs_exist_ok=True)
            
            # Use incomplete functions (don't use complete as baseline)
            source_path = temp_repo / source_file
            
            # Inject with backup using function ID
            success, backup_path = self.injector.inject_with_backup(
                source_path, completion, function_id
            )
            
            if not success:
                return False, ["Code injection failed"]
            
            # Make run_tests.sh executable
            import os
            run_tests_path = temp_repo / "run_tests.sh"
            if run_tests_path.exists():
                os.chmod(run_tests_path, 0o755)
            
            # Test in Docker with docker cp approach
            container_name = f"t3_cpp-{repo_name.lower()}"
            import subprocess as sp
            
            # Ensure Docker image exists; if missing, attempt to build from repo
            img_check = sp.run(["docker", "images", "-q", container_name], capture_output=True, text=True)
            if not img_check.stdout.strip():
                # Attempt to build the image from the repository path
                repo_path_local = repo_path
                build = sp.run(["docker", "build", "-t", container_name, str(repo_path_local)], capture_output=True, text=True)
                if build.returncode != 0:
                    raise RuntimeError(f"docker build failed: {build.stderr.strip()}")

            # Create container with command to run tests
            created = sp.run([
                "docker", "create",
                "--rm",               # Auto-remove on exit
                "-w", "/workspace",
                container_name,
                "/bin/bash", "-c", "bash run_tests.sh"
            ], capture_output=True, text=True)

            container_id = created.stdout.strip()
            if not container_id:
                err_msg = created.stderr.strip() or 'docker create returned empty container id'
                raise RuntimeError(f"docker create failed: {err_msg}")

            # Copy files to container with retries to handle transient docker issues
            import time
            cp_attempts = 3
            cp_success = False
            last_cp_exception = None
            for attempt in range(1, cp_attempts + 1):
                try:
                    sp.run(["docker", "cp", f"{temp_repo}/.", f"{container_id}:/workspace"], check=True)
                    cp_success = True
                    break
                except Exception as e:
                    last_cp_exception = e
                    # If it's the last attempt, break and propagate
                    if attempt == cp_attempts:
                        break
                    time.sleep(1 * attempt)

            if not cp_success:
                # Attempt to cleanup container before raising
                try:
                    sp.run(["docker", "rm", "-f", container_id], capture_output=True)
                except Exception:
                    pass
                raise RuntimeError(f"docker cp failed after {cp_attempts} attempts: {last_cp_exception}")

            # Start container and wait for completion
            result = subprocess.run([
                "docker", "start", "-a", container_id
            ], capture_output=True, timeout=600, text=True)

            # Cleanup
            try:
                sp.run(["docker", "rm", container_id], capture_output=True)
            except Exception:
                pass

            stdout = result.stdout or ""
            stderr = result.stderr or ""
            combined = stdout + "\n" + stderr

            # Always prepare full test artifact
            full_results = {
                'stdout': stdout,
                'stderr': stderr,
                'returncode': result.returncode,
                # 'logs' contains parsed informational and test lines for PRM training
                # 'errors' will only contain true error markers or be populated when the run failed
                # (keeps semantics clear: logs != errors)
                'errors': []
            }

            # Populate logs (comprehensive parsed output) for PRM training
            try:
                logs = self._parse_full_output(combined)
            except Exception:
                logs = []
            full_results['logs'] = logs

            # Populate errors conservatively: only when returncode != 0 (fail) or when
            # explicit error/linker markers appear in logs. This prevents build/test
            # informational lines from being mislabeled as 'errors' on successful runs.
            errors_list = []
            try:
                if result.returncode != 0:
                    # On failure, extract compilation/linker errors (full, untruncated)
                    errors_list = self._parse_compilation_errors(combined, for_llm=False)
                    # If that returns empty, fall back to logs
                    if not errors_list:
                        errors_list = logs
                else:
                    # On success, only surface explicit error-like markers if present
                    lower_logs = [l.lower() for l in logs]
                    error_markers = ['error:', 'undefined reference', 'ld:', 'collect2:', 'ld returned']
                    for l in logs:
                        if any(m in l.lower() for m in error_markers):
                            errors_list.append(l)
            except Exception:
                errors_list = []

            full_results['errors'] = errors_list

            # Prepare filtered output for LLM (compilation-only, truncated)
            filtered = []
            try:
                filtered = self._parse_compilation_errors(combined, for_llm=True)
            except Exception:
                filtered = []

            # Extract some environment metadata for reproducibility
            import time
            cmake_compiler = None
            # Try to parse common CMake compiler identification lines
            for line in (stdout + '\n' + stderr).splitlines():
                if '-- The CXX compiler identification is' in line:
                    cmake_compiler = line.split('is', 1)[-1].strip()
                    break

            env = {
                'container': container_name,
                'returncode': result.returncode,
                'temp_id': temp_id,
                'cmake_compiler': cmake_compiler,
                'timestamp': time.time()
            }

            # If the run failed
            if result.returncode != 0:
                if filter_test_output:
                    # For LLM: prefer showing compilation errors only; if none, indicate tests failed
                    if filtered:
                        return False, {'filtered': filtered, 'full_test_results': full_results, 'env': env}
                    else:
                        # Compiled but tests failed (don't leak test specifics)
                        return True, {'filtered': [], 'full_test_results': full_results, 'env': env}
                else:
                    # For PRM: return full results and mark failure
                    return False, {'filtered': filtered, 'full_test_results': full_results, 'env': env}

            # Success (returncode == 0)
            return True, {'filtered': filtered, 'full_test_results': full_results, 'env': env}
            
        except Exception as e:
            return False, [f"Docker test exception: {str(e)}"]
        finally:
            # Cleanup
            shutil.rmtree(temp_repo, ignore_errors=True)
    
    def _parse_compilation_errors(self, output: str, for_llm: bool = True) -> list:
        """Extract ONLY compilation errors from Docker output (NOT test results)
        
        Args:
            output: Docker output
            for_llm: If True, truncate to ~200-500 tokens. If False, return all errors.
        """
        errors = []
        lines = output.split('\n')
        
        # Comprehensive test framework patterns to exclude
        test_patterns = [
            # CTest/CMake test output
            'Test project', 'tests passed', 'tests failed', 'Test #', 'Total Test time',
            'Start testing', 'CTest', 'ctest', 'Testing/Temporary',
            # Google Test
            'RUN', 'OK', 'PASSED', 'FAILED', '[==========]', '[----------]', '[ RUN      ]',
            '[       OK ]', '[  PASSED  ]', '[  FAILED  ]', 'Ran ', 'tests from',
            # Catch2
            'test cases:', 'assertions:', 'All tests passed', 'test case failed',
            # Boost.Test  
            'Test suite', 'Test case', 'Entering test', 'Leaving test',
            # Generic test indicators
            'Passed', 'Failed', '% tests', 'Subprocess aborted', 'The following tests',
            'Assertion', 'ASSERT', 'EXPECT', 'CHECK', 'REQUIRE',
            # Test execution markers
            '===', '***', '---', 'Running', 'Executing'
        ]
        
        # Compilation/build error markers (what we WANT to capture)
        error_markers = ['error:', 'undefined reference', 'no matching function', 
                        'no member named', 'incomplete type', 'cannot convert',
                        'expected', 'declaration', 'redefinition']
        
        for line in lines:
            line_lower = line.lower()
            
            # STRICT: Skip any line matching test patterns
            if any(pattern.lower() in line_lower for pattern in test_patterns):
                continue
            
            # Only capture actual compilation/linking errors
            if any(marker in line for marker in error_markers):
                # Triple-check it's not test output
                if not any(pattern.lower() in line_lower for pattern in test_patterns):
                    errors.append(line.strip())
            
            # Capture make/cmake build errors (not test errors)
            elif ('make:' in line_lower or 'cmake' in line_lower) and 'error' in line_lower:
                if 'test' not in line_lower:  # Extra safety
                    errors.append(line.strip())
        
        # If no errors found, check for linker errors
        if not errors:
            for line in lines:
                if any(marker in line.lower() for marker in ['ld:', 'collect2:', 'ld returned']):
                    if not any(pattern.lower() in line.lower() for pattern in test_patterns):
                        errors.append(line.strip())
        
        # Truncate for LLM to save tokens (~200-500 tokens = ~3-8 errors)
        if for_llm:
            # Estimate: average error is ~60-80 tokens, so 5-7 errors ≈ 300-500 tokens
            return self._truncate_to_token_budget(errors, max_tokens=500)
        
        return errors  # Full array for trajectory
    
    def _truncate_to_token_budget(self, errors: list, max_tokens: int = 500) -> list:
        """Truncate error list to fit within token budget
        
        Args:
            errors: List of error strings
            max_tokens: Maximum tokens to use (~200-500 recommended)
        
        Returns:
            Truncated error list that fits within token budget
        """
        # Rough estimate: 1 token ≈ 4 characters for English text
        # For error messages with code, use 1 token ≈ 3 characters (more technical)
        chars_per_token = 3
        max_chars = max_tokens * chars_per_token
        
        truncated = []
        total_chars = 0
        
        for error in errors:
            error_chars = len(error)
            if total_chars + error_chars <= max_chars:
                truncated.append(error)
                total_chars += error_chars
            else:
                # Add partial error if there's room
                remaining = max_chars - total_chars
                if remaining > 100:  # Only add if meaningful
                    truncated.append(error[:remaining] + '...')
                break
        
        return truncated
    
    def _parse_full_output(self, output: str) -> list:
        """Extract ALL relevant output including test failures (for PRM training)"""
        lines = output.split('\n')
        relevant = []
        
        # Capture compilation errors
        error_markers = ['error:', 'undefined reference', 'no matching function', 
                        'no member named', 'incomplete type', 'cannot convert']
        
        # Capture test results
        test_markers = ['tests passed', 'tests failed', 'FAILED', 'PASSED', 
                       'Assertion', 'Expected', 'Actual']
        
        for line in lines:
            line_stripped = line.strip()
            if not line_stripped:
                continue
            
            # Capture compilation errors
            if any(marker in line for marker in error_markers):
                relevant.append(line_stripped)
            # Capture test results
            elif any(marker in line for marker in test_markers):
                relevant.append(line_stripped)
            # Capture test summary lines
            elif '%' in line and 'test' in line.lower():
                relevant.append(line_stripped)
        
        return relevant[:50]  # More lines for PRM training