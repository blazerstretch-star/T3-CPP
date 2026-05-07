"""
Agentic Tools for t3_cpp
Provides 4 core tools for autonomous agent research and problem-solving
"""

import os
import re
import json
import requests
import tempfile
import subprocess
import fnmatch
from pathlib import Path
from typing import Dict, List, Any, Optional
import shutil
import logging


class AgenticTools:
    """
    Core tools for agentic C++ code generation
    """
    
    def __init__(self, repo_path: str, enable_web: bool = True, 
                 enable_cpp_docs: bool = True, enable_test: bool = True,
                 enable_file_read: bool = True, mask_function_id: str = None,
                 current_task: dict = None):
        self.repo_path = os.path.abspath(repo_path)
        # CRITICAL: Restrict to context/ folder only (prevent seeing answers in src/)
        self.context_path = os.path.join(self.repo_path, 'context')
        # Initialize logger for terminal + log output
        self.logger = logging.getLogger(__name__)
        # Configure basic logging only if not already configured by caller
        if not logging.getLogger().handlers:
            logging.basicConfig(level=logging.INFO)

        # Strict enforcement: context/ MUST exist. Fail-fast to avoid accidental
        # leakage or wider repo access when context is missing.
        if not os.path.exists(self.context_path):
            msg = (
                f"Missing required 'context/' directory in repository: {self.repo_path}.\n"
                "Ensure benchmarks/<repo>/context exists and contains the allowed files."
            )
            print(msg)
            self.logger.error(msg)
            raise ValueError(msg)
        else:
            ok_msg = f"context folder exists ok: {self.context_path}"
            print(ok_msg)
            self.logger.info(ok_msg)
        
        self.enable_web = enable_web
        self.enable_cpp_docs = enable_cpp_docs
        self.enable_test = enable_test
        self.enable_file_read = enable_file_read
        self.mask_function_id = mask_function_id
        self.current_task = current_task  # Task metadata with signature, file path, etc.
        
        # Safety: Ensure repo_path exists
        if not os.path.exists(self.repo_path):
            raise ValueError(f"Repository path does not exist: {self.repo_path}")
    
    def search_repo_code(self, query: str, max_results: int = 10, start_index: int = 0) -> List[Dict[str, str]]:
        """
        Search codebase for patterns, function usage, or examples
        
        Args:
            query: Search query (supports regex)
            max_results: Maximum number of results to return (default: 10)
            start_index: Starting index for pagination (default: 0)
        
        Returns:
            List of matches with file path and code snippet
        """
        # Robustness: LLMs sometimes pass numeric args as strings (e.g. "5" instead of 5)
        max_results = int(max_results)
        start_index = int(start_index)
        
        results = []
        allowed_extensions = (
            # C
            '.c', '.i',

            # C++
            '.cpp', '.cc', '.cxx', '.c++', '.cp',
            '.tpp', '.ipp', '.inl',

            # Headers
            '.h', '.hh', '.hpp', '.hxx', '.h++',
            '.tcc', '.inc',

            # Modules
            '.ixx', '.cppm', '.cxxm', '.mpp', '.mxx',

            # Preprocessed
            '.ii', '.mi',

            # Objective-C / Obj-C++
            '.m', '.mm',

            # CUDA
            '.cu', '.cuh',

            # Misc
            '.idl', '.def', '.odl', '.pch', '.gch'
        )
        
        try:
            pattern = re.compile(query, re.IGNORECASE)
        except re.error:
            # If regex is invalid, treat as literal string
            pattern = re.compile(re.escape(query), re.IGNORECASE)
        
        for root, _, files in os.walk(self.context_path):
            # Security: Prevent directory traversal
            if not os.path.abspath(root).startswith(self.context_path):
                continue
            
            for filename in files:
                if not filename.endswith(allowed_extensions):
                    continue
                
                filepath = os.path.join(root, filename)
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                    
                    if self.mask_function_id:
                        content = self._mask_function(content, self.mask_function_id)
                        
                    if pattern.search(content):
                        snippet = self._extract_context(content, pattern)
                        # Return path relative to context_path for consistency
                        rel_path = os.path.relpath(filepath, self.context_path)
                        results.append({
                            'file': rel_path,
                            'snippet': snippet
                        })
                        
                        # Collect all results first for pagination
                        if len(results) >= max_results + start_index + 50:
                            break
                except Exception:
                    continue
        
        # Apply pagination
        total_found = len(results)
        results = results[start_index:start_index + max_results]
        
        # Add metadata about pagination only on follow-up pages
        if start_index > 0:
            results.append({
                'file': '__pagination__',
                'type': '__pagination__',
                'snippet': f'Showing results {start_index+1}-{start_index+len(results)} of {total_found} total matches'
            })
        
        return results
    
    def search_web(self, query: str) -> List[Dict[str, str]]:
        """
        Search internet for documentation, tutorials, or examples
        
        Args:
            query: Search query
        
        Returns:
            List of search results with title and URL
        """
        if not self.enable_web:
            return [{"error": "Web search is disabled"}]
        
        try:
            # Use DuckDuckGo HTML search (no API key needed)
            url = "https://html.duckduckgo.com/html/"
            params = {'q': query}
            headers = {'User-Agent': 'Mozilla/5.0'}
            
            response = requests.get(url, params=params, headers=headers, timeout=10)
            response.raise_for_status()
            
            soup = BeautifulSoup(response.text, 'html.parser')
            results = []
            
            for result in soup.find_all('a', class_='result__a', limit=5):
                title = result.get_text(strip=True)
                href = result.get('href', '')
                
                if title and href:
                    results.append({
                        'title': title,
                        'url': href
                    })
            
            return results if results else [{"message": "No results found"}]
            
        except Exception as e:
            return [{"error": f"Web search failed: {str(e)}"}]
    
    def get_cpp_docs(self, topic: str) -> str:
        """
        Get C++ standard library documentation from cppreference.com
        Agent can search for any C++ topic, function, or container
        
        Args:
            topic: C++ topic (e.g., "vector", "std::sort", "algorithm", "iterator")
        
        Returns:
            Documentation text (first 2000 characters)
        """
        if not self.enable_cpp_docs:
            return "C++ documentation access is disabled"
        
        try:
            # Clean topic: remove std:: prefix for URL
            clean_topic = topic.replace('std::', '').replace('::', '/')
            
            # Try direct page first (most common case)
            direct_urls = [
                f"https://en.cppreference.com/w/cpp/container/{clean_topic}",
                f"https://en.cppreference.com/w/cpp/algorithm/{clean_topic}",
                f"https://en.cppreference.com/w/cpp/numeric/{clean_topic}",
                f"https://en.cppreference.com/w/cpp/string/{clean_topic}",
                f"https://en.cppreference.com/w/cpp/utility/{clean_topic}",
                f"https://en.cppreference.com/w/cpp/{clean_topic}"
            ]
            
            headers = {'User-Agent': 'Mozilla/5.0'}
            
            for url in direct_urls:
                try:
                    response = requests.get(url, headers=headers, timeout=5)
                    if response.status_code == 200:
                        soup = BeautifulSoup(response.text, 'html.parser')
                        content_div = soup.find('div', id='mw-content-text')
                        
                        if content_div:
                            # Remove script and style
                            for script in content_div(['script', 'style', 'table']):
                                script.decompose()
                            
                            text = content_div.get_text(separator='\n', strip=True)
                            if len(text) > 100:  # Valid content
                                return f"Documentation for {topic}:\n{text[:2000]}"
                except:
                    continue
            
            # Fallback: web search for the topic
            return f"Could not find direct docs for '{topic}'. Try web search or check spelling."
            
        except Exception as e:
            return f"C++ docs error: {str(e)}"
    
    def test_code_snippet(self, code: str) -> Dict[str, Any]:
        """
        Test if code compiles in Docker with full build environment.
        
        Args:
            code: Function body code (no signature)
        
        Returns:
            Dict with 'compiles' (bool), 'errors' (list), and 'full_test_results' (full output)
            The 'errors' field contains 2 lines from start + 2 lines from end for LLM view
        """
        if not self.enable_test:
            return {"compiles": False, "errors": ["Code testing is disabled"]}
        
        if not self.current_task:
            return {"compiles": False, "errors": ["No task context available"]}

        
        try:
            from code_and_train_ds.t3_cpp.core.robust_injector import RobustDockerEvaluator
            
            src_file = self.current_task.get('completion_path', '')
            function_id = self.current_task.get('function_id', '')
            
            if not src_file or not function_id:
                return {"compiles": False, "errors": ["Missing completion_path or function_id"]}
            
            # Extract repo name from repo_path (e.g., benchmarks/adhesion -> adhesion)
            repo_name = Path(self.repo_path).name
            source_file = src_file  # Already relative to repo (e.g., src/file.cpp)
            
            # Use Docker evaluator for compilation check
            evaluator = RobustDockerEvaluator(Path(self.repo_path).parent)
            
            # Test compilation (evaluator now returns tuple: success, payload)
            # Use filter_test_output=False to get full test output
            success, payload = evaluator.inject_and_test_docker(
                repo_name=repo_name,
                source_file=source_file,
                completion=code,
                function_id=function_id,
                filter_test_output=False  # Get full test output
            )

            # Backwards-compatible: payload might be a list of errors (older behavior)
            if isinstance(payload, list):
                return {"compiles": success, "tests_pass": None, "test_summary": "COMPILATION_FAILED", "errors": payload}

            # Structured payload: get full test results
            filtered = payload.get('filtered', []) if isinstance(payload, dict) else []
            full = payload.get('full_test_results') if isinstance(payload, dict) else None

            # FIX: Use returncode as ground truth for compilation status
            # returncode meanings: 0=success, 2=compilation failed, 8=tests failed (compiled OK)
            compiles = None
            tests_pass = None
            test_summary = None
            
            if isinstance(full, dict) and full.get('returncode') is not None:
                rc = full.get('returncode')
                # Ground truth: returncode tells us what actually happened
                compiles = (rc != 2)  # Compiled successfully if returncode is NOT 2
                tests_pass = (rc == 0)  # Tests passed only if returncode is 0
                test_summary = 'PASSED' if tests_pass else 'FAILED'
            else:
                # Fallback: if no returncode available, trust filtered errors
                compiles = len(filtered) == 0
                test_summary = 'COMPILATION_FAILED' if not compiles else None

            # Filter test output for LLM: 2 lines from start + 2 lines from end
            llm_errors = self._filter_test_output_for_llm(filtered)

            result = {
                'compiles': compiles,
                'tests_pass': tests_pass,
                'test_summary': test_summary,
                'errors': llm_errors  # Filtered for LLM (2 start + 2 end)
            }

            # Preserve full test artifact for internal storage
            if full is not None:
                result['full_test_results'] = full

            # Also include env metadata if present (useful for debugging)
            if isinstance(payload, dict) and 'env' in payload:
                result['test_env'] = payload['env']

            return result
                    
        except Exception as e:
            return {"compiles": False, "errors": [f"Docker test failed: {str(e)}"]}
    
    def _filter_test_output_for_llm(self, errors: List[str]) -> List[str]:
        """
        Filter test output to show only 2 lines from start and 2 lines from end.
        
        Args:
            errors: Full error/test output lines
        
        Returns:
            Filtered list with 2 start + 2 end lines (or all if <= 4 lines)
        """
        if not errors:
            return []
        
        if len(errors) <= 4:
            return errors
        
        # Take 2 from start and 2 from end
        filtered = errors[:2] + [f"... ({len(errors) - 4} lines omitted) ..."] + errors[-2:]
        return filtered
    
    def read_files(self, paths: List[str], max_chars: int = 10000, start_char: int = 0) -> List[Dict[str, Any]]:
        """
        Read one or multiple files (READ-ONLY)
        
        Args:
            paths: List of file paths (relative to repo or absolute)
            max_chars: Maximum characters to read per file (default: 10000 = ~2500 tokens)
            start_char: Starting character position for partial reads (default: 0)
        
        Returns:
            List of dicts with 'path', 'content', 'truncated', 'error'
        """
        if not self.enable_file_read:
            return [{"error": "File reading is disabled"}]
        
        # Robustness: LLMs sometimes pass paths as a string instead of a list
        # e.g. "['file1.cc']" or "file1.cc" instead of ["file1.cc"]
        if isinstance(paths, str):
            # Try to parse as JSON list first (e.g. '["file1.cc", "file2.cc"]')
            import ast as _ast
            try:
                parsed = json.loads(paths)
                if isinstance(parsed, list):
                    paths = [str(p) for p in parsed]
                else:
                    paths = [paths]
            except (json.JSONDecodeError, ValueError):
                # Try Python literal (e.g. "['file1.cc']")
                try:
                    parsed = _ast.literal_eval(paths)
                    if isinstance(parsed, (list, tuple)):
                        paths = [str(p) for p in parsed]
                    else:
                        paths = [paths]
                except (ValueError, SyntaxError):
                    # Plain string — treat as single file path
                    paths = [paths]
        
        # Coerce max_chars and start_char to int (LLMs sometimes pass strings)
        max_chars = int(max_chars)
        start_char = int(start_char)
        
        results = []
        
        for path in paths:
            full_path = self._resolve_path(path)
            
            if not self._is_safe_path(full_path):
                results.append({'path': path, 'error': 'Path outside repository'})
                continue
            
            if not os.path.exists(full_path):
                results.append({'path': path, 'error': 'File not found'})
                continue
            
            try:
                with open(full_path, 'r', encoding='utf-8', errors='ignore') as f:
                    full_content = f.read()

                file_size = len(full_content)
                content = full_content[start_char:start_char + max_chars]

                # If reading from the middle of a file, include the immediately
                # preceding section comment line to preserve navigational context.
                if start_char > 0 and content:
                    current_line_start = full_content.rfind('\n', 0, start_char) + 1
                    if current_line_start < start_char:
                        previous_line_end = current_line_start - 1
                        previous_line_start = full_content.rfind('\n', 0, previous_line_end) + 1
                        previous_line = full_content[previous_line_start:previous_line_end + 1].strip()

                        if previous_line.startswith("// SECTION"):
                            section_prefix = full_content[previous_line_start:current_line_start]
                            if section_prefix and section_prefix not in content:
                                remaining = max(0, max_chars - len(section_prefix))
                                content = section_prefix + full_content[start_char:start_char + remaining]

                truncated = (start_char + len(content)) < file_size
                
                if self.mask_function_id:
                    content = self._mask_function(content, self.mask_function_id)
                
                results.append({
                    'path': path,
                    'content': content,
                    'truncated': truncated,
                    'size': file_size,
                    'start_char': start_char,
                    'end_char': start_char + len(content)
                })
            except Exception as e:
                results.append({'path': path, 'error': str(e)})
        
        return results
    
    def list_directory(self, path: str = ".", max_depth: Optional[int] = 0) -> Dict[str, Any]:
        """
        List directory contents recursively (READ-ONLY)
        
        Args:
            path: Directory path (relative to repo)
            max_depth: Maximum recursion depth (default: 0 = only immediate children)
        
        Returns:
            Dict with 'tree' (list of paths) and 'error'
        """
        if not self.enable_file_read:
            return {"error": "File reading is disabled"}
        
        # Robustness: LLMs sometimes pass max_depth as a string (e.g. "1" instead of 1)
        if max_depth is not None:
            max_depth = int(max_depth)
        
        full_path = self._resolve_path(path)
        
        if not self._is_safe_path(full_path):
            return {'error': 'Path outside repository'}
        
        if not os.path.exists(full_path):
            hint = self._get_top_level_dirs_hint()
            return {'error': f'Directory not found: \'{path}\'. {hint}'}
        
        if not os.path.isdir(full_path):
            return {'error': 'Not a directory'}
        
        try:
            tree = []
            self._walk_directory(full_path, tree, max_depth, 0, "")
            # Truncate tree if too large (max ~100 entries = ~500 tokens)
            if len(tree) > 100:
                tree = tree[:100]
                tree.append(f"... ({len(tree) - 100} more entries truncated)")
            return {'tree': tree, 'count': len(tree)}
        except Exception as e:
            return {'error': str(e)}
    
    def search_files(self, query: str, path: str = ".", 
                    max_depth: Optional[int] = None,
                    case_sensitive: bool = False,
                    max_results: int = 50,
                    start_index: int = 0) -> List[Dict[str, str]]:
        """
        Search for files by name (fuzzy matching) (READ-ONLY)
        
        Args:
            query: Search query (supports wildcards)
            path: Starting directory
            max_depth: Maximum search depth
            case_sensitive: Case-sensitive matching
            max_results: Maximum number of results (default: 50)
            start_index: Starting index for pagination (default: 0)
        
        Returns:
            List of matching files with paths and types
        """
        if not self.enable_file_read:
            return [{"error": "File reading is disabled"}]
        
        # Robustness: LLMs sometimes pass numeric args as strings
        max_results = int(max_results)
        start_index = int(start_index)
        if max_depth is not None:
            max_depth = int(max_depth)
        
        full_path = self._resolve_path(path)
        
        if not self._is_safe_path(full_path):
            return [{'error': 'Path outside repository'}]
        
        if not os.path.exists(full_path):
            hint = self._get_top_level_dirs_hint()
            return [{'error': f'Directory not found: \'{path}\'. {hint}'}]
        
        results = []
        pattern = query if case_sensitive else query.lower()
        
        try:
            for root, dirs, files in os.walk(full_path):
                depth = root[len(full_path):].count(os.sep)
                if max_depth is not None and depth > max_depth:
                    dirs.clear()
                    continue
                
                for dirname in dirs:
                    name = dirname if case_sensitive else dirname.lower()
                    if self._fuzzy_match(pattern, name):
                        abs_path = os.path.join(root, dirname)
                        # Return path relative to context_path for consistency
                        rel_path = os.path.relpath(abs_path, self.context_path)
                        results.append({'type': 'directory', 'path': rel_path, 'name': dirname})
                
                for filename in files:
                    name = filename if case_sensitive else filename.lower()
                    if self._fuzzy_match(pattern, name):
                        abs_path = os.path.join(root, filename)
                        # Return path relative to context_path for consistency
                        rel_path = os.path.relpath(abs_path, self.context_path)
                        results.append({'type': 'file', 'path': rel_path, 'name': filename})
                
                if len(results) >= max_results + start_index + 100:  # Collect extra for pagination
                    break
            
            # Apply pagination
            total_found = len(results)
            sorted_results = sorted(results, key=lambda x: (x['type'], x['name']))
            paginated = sorted_results[start_index:start_index + max_results]
            
            # Add pagination info
            if start_index > 0 or total_found > start_index + max_results:
                paginated.append({
                    'type': '__pagination__',
                    'path': f'Showing {start_index+1}-{start_index+len(paginated)} of {total_found} matches',
                    'name': 'Use start_index parameter to see more'
                })
            
            return paginated
        except Exception as e:
            return [{'error': str(e)}]
    
    def get_tool_definitions(self, model_name: str = "") -> List[Dict[str, Any]]:
        """
        Get OpenAI-style tool definitions for LLM
        
        Args:
            model_name: Model name for compatibility adjustments (e.g., "deepseek-ai/deepseek-r1-distill-qwen-32b")
        
        Returns:
            List of tool definitions in OpenAI function calling format
        """
        tools = []
        
        # Check if model requires strict OpenAI spec compliance (e.g., deepseek models)
        requires_strict_spec = any(keyword in model_name.lower() for keyword in ['deepseek', 'nvidia'])
        
        # Tool 1: search_repo_code
        tools.append({
            "type": "function",
            "function": {
                "name": "search_repo_code",
                "description": "Search the repository codebase for code patterns, function usage, class definitions, or examples. Use this to understand existing code structure and find similar implementations. Supports pagination.",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "query": {
                            "type": "string",
                            "description": "Search query (supports regex). Examples: 'BoundingBox3D', 'Point3D.*operator', 'namespace.*OrthoTree'"
                        },
                        "max_results": {
                            "type": "integer",
                            "description": "Maximum results to return (default: 10, max: 50)"
                        },
                        "start_index": {
                            "type": "integer",
                            "description": "Starting index for pagination (default: 0). Use to see more results."
                        }
                    },
                    "required": ["query"]
                }
            }
        })
        
        # Tool 2: search_web
        if self.enable_web:
            tools.append({
                "type": "function",
                "function": {
                    "name": "search_web",
                    "description": "Search the internet for documentation, tutorials, algorithm explanations, or code examples. Use this when you need external knowledge not available in the repository.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "query": {
                                "type": "string",
                                "description": "Search query. Examples: '3D bounding box center calculation', 'C++ Point3D implementation', 'octree algorithm'"
                            }
                        },
                        "required": ["query"]
                    }
                }
            })
        
        # Tool 3: get_cpp_docs
        if self.enable_cpp_docs:
            tools.append({
                "type": "function",
                "function": {
                    "name": "get_cpp_docs",
                    "description": "Get C++ standard library documentation from cppreference.com. Use this to understand standard library functions, containers, or algorithms.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "topic": {
                                "type": "string",
                                "description": "C++ topic or function name. Examples: 'std::vector', 'std::algorithm', 'std::sqrt', 'iterator'"
                            }
                        },
                        "required": ["topic"]
                    }
                }
            })
        
        # Tool 4: test_code_snippet
        if self.enable_test:
            tools.append({
                "type": "function",
                "function": {
                    "name": "test_code_snippet",
                    "description": "Test if C++ code compiles. COMPLETE standard library pre-loaded: all containers (vector, map, set, unordered_map, queue, stack, deque, list), all algorithms (sort, find, binary_search), all utilities (string, pair, tuple, unique_ptr). Use any STL freely without includes.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "code": {
                                "type": "string",
                                "description": "C++ code to test. Use any STL: std::map<int,int> m; std::priority_queue<int> pq; std::unordered_set<string> s; All work without includes."
                            },
                            "includes": {
                                "type": "string",
                                "description": "RARELY NEEDED. Only for non-standard libraries. Standard library is complete."
                            }
                        },
                        "required": ["code"]
                    }
                }
            })
        
        # Tool 5: read_files
        if self.enable_file_read:
            tools.append({
                "type": "function",
                "function": {
                    "name": "read_files",
                    "description": "Read one or multiple files. Working directory is 'context/'. Use relative paths like 'adhesion/edge_count.cc'. Supports partial reads for large files.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "paths": {
                                "type": "array",
                                "items": {"type": "string"},
                                "description": "List of file paths relative to context/. Examples: ['adhesion/edge_count.cc', 'cgal_base.hh']"
                            },
                            "max_chars": {
                                "type": "integer",
                                "description": "Max characters per file (default: 10000 = ~2500 tokens). Reduce for large files."
                            },
                            "start_char": {
                                "type": "integer",
                                "description": "Starting character position (default: 0). Use to read later parts of large files."
                            }
                        },
                        "required": ["paths"]
                    }
                }
            })
        
        # Tool 6: list_directory
        if self.enable_file_read:
            list_dir_params = {
                "type": "object",
                "properties": {
                    "path": {
                        "type": "string",
                        "description": "Directory path relative to context/ (default: '.')"
                    },
                    "max_depth": {
                        "type": "integer",
                        "description": "Maximum recursion depth (optional)"
                    }
                }
            }
            # Add required field for strict spec compliance (deepseek, nvidia models)
            if requires_strict_spec:
                list_dir_params["required"] = []
            
            tools.append({
                "type": "function",
                "function": {
                    "name": "list_directory",
                    "description": "List directory contents. Working directory is 'context/'. Use '.' for current directory or relative paths like 'adhesion/'.",
                    "parameters": list_dir_params
                }
            })
        
        # Tool 7: search_files
        if self.enable_file_read:
            tools.append({
                "type": "function",
                "function": {
                    "name": "search_files",
                    "description": "Search for files by name. Working directory is 'context/'. Returns paths relative to context/. Supports pagination.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "query": {
                                "type": "string",
                                "description": "Search query. Examples: '*.cpp', 'edge_count.cc', 'cgal*'"
                            },
                            "path": {
                                "type": "string",
                                "description": "Starting directory relative to context/ (default: '.')"
                            },
                            "max_depth": {
                                "type": "integer",
                                "description": "Maximum search depth (optional)"
                            },
                            "case_sensitive": {
                                "type": "boolean",
                                "description": "Case-sensitive matching (default: false)"
                            },
                            "max_results": {
                                "type": "integer",
                                "description": "Maximum results to return (default: 50)"
                            },
                            "start_index": {
                                "type": "integer",
                                "description": "Starting index for pagination (default: 0)"
                            }
                        },
                        "required": ["query"]
                    }
                }
            })

        return tools
    
    def _extract_context(self, content: str, pattern: re.Pattern, context_lines: int = 3) -> str:
        """
        Extract code snippet with surrounding context
        
        Args:
            content: File content
            pattern: Compiled regex pattern
            context_lines: Number of lines before/after match
        
        Returns:
            Code snippet with context
        """
        lines = content.split('\n')
        
        for i, line in enumerate(lines):
            if pattern.search(line):
                start = max(0, i - context_lines)
                end = min(len(lines), i + context_lines + 1)
                
                snippet_lines = lines[start:end]
                # Add line numbers for clarity
                numbered = [f"{start + j + 1:4d} | {l}" for j, l in enumerate(snippet_lines)]
                
                return '\n'.join(numbered)
        
        return "Match found but context extraction failed"
    
    def _resolve_path(self, path: str) -> str:
        """Resolve path relative to context folder - context is the base directory"""
        # Always resolve relative to context_path (model's working directory)
        # This makes paths like 'adhesion/edge_count.cc' work naturally
        return os.path.abspath(os.path.join(self.context_path, path))
    
    def _is_safe_path(self, path: str) -> bool:
        """Check if path is within context folder (prevent seeing answers)"""
        return os.path.abspath(path).startswith(self.context_path)
    
    def _walk_directory(self, path: str, tree: List[str], max_depth: Optional[int], 
                       current_depth: int, prefix: str, base_path: str = None):
        """Recursively walk directory"""
        if max_depth is not None and current_depth > max_depth:
            return
        
        if base_path is None:
            base_path = path
        
        skip_dirs = {'build', 'dist', 'node_modules', '.git', '__pycache__', 'CMakeFiles'}
        
        try:
            items = sorted(os.listdir(path))
            dirs = [d for d in items if os.path.isdir(os.path.join(path, d)) and d not in skip_dirs]
            files = [f for f in items if os.path.isfile(os.path.join(path, f))]
            
            for i, dirname in enumerate(dirs):
                is_last_dir = (i == len(dirs) - 1) and len(files) == 0
                connector = "└── " if is_last_dir else "├── "
                tree.append(f"{prefix}{connector}{dirname}/")
                
                new_prefix = prefix + ("    " if is_last_dir else "│   ")
                self._walk_directory(
                    os.path.join(path, dirname), 
                    tree, max_depth, current_depth + 1, new_prefix, base_path
                )
            
            for i, filename in enumerate(files):
                is_last = i == len(files) - 1
                connector = "└── " if is_last else "├── "
                tree.append(f"{prefix}{connector}{filename}")
        except PermissionError:
            pass
    
    def _get_top_level_dirs_hint(self) -> str:
        """Return a hint listing top-level directories in context_path"""
        try:
            entries = sorted(os.listdir(self.context_path))
            dirs = [d for d in entries if os.path.isdir(os.path.join(self.context_path, d))]
            if dirs:
                return f"Available top-level directories: {', '.join(dirs)}. Use full paths like \'{dirs[0]}/...\'."
            return "No subdirectories found. Use '.' to list root."
        except Exception:
            return "Use list_directory('.') to see available paths."
    
    def _fuzzy_match(self, pattern: str, text: str) -> bool:
        """Fuzzy match with wildcard support"""
        if '*' in pattern or '?' in pattern:
            return fnmatch.fnmatch(text, pattern)
        return pattern in text
    
    def _mask_function(self, content: str, function_id: str) -> str:
        """Mask ALL function implementations between FUNCTION_ID markers"""
        # Mask all functions marked with FUNCTION_ID comments
        pattern = r'// FUNCTION_ID: ([^\n]+) - START.*?// FUNCTION_ID: \1 - END'
        replacement = r'// FUNCTION_ID: \1 - START\n  // TODO: Implement this function\n  // FUNCTION_ID: \1 - END'
        
        return re.sub(pattern, replacement, content, flags=re.DOTALL)


# Convenience function for tool execution
def execute_tool(tools: AgenticTools, tool_name: str, arguments: Dict[str, Any]) -> Any:
    """
    Execute a tool by name with given arguments
    
    Args:
        tools: AgenticTools instance
        tool_name: Name of tool to execute
        arguments: Tool arguments as dict
    
    Returns:
        Tool execution result
    """
    if not hasattr(tools, tool_name):
        return {"error": f"Unknown tool: {tool_name}"}
    
    try:
        method = getattr(tools, tool_name)
        return method(**arguments)
    except Exception as e:
        return {"error": f"Tool execution failed: {str(e)}"}
