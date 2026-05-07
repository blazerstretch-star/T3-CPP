#!/usr/bin/env python3
"""
API Adapter for Agentic Inference
Adapts run_agentic_inference.py to work with any OpenAI-compatible inference provider
"""

import os
import sys
import json
import argparse
import requests
from pathlib import Path

# ── INFERENCE PROVIDER CONFIGURATION ─────────────────────────────────────────
# Set this to your OpenAI-compatible inference endpoint.
# Examples:
#   NVIDIA NIM:  https://integrate.api.nvidia.com/v1/chat/completions
#   OpenAI:      https://api.openai.com/v1/chat/completions
#   vLLM local:  http://localhost:8000/v1/chat/completions
inference_provider = os.environ.get(
    "INFERENCE_PROVIDER",
    "https://integrate.api.nvidia.com/v1/chat/completions"  # default
)

# Add parent directory to path
sys.path.append(str(Path(__file__).parent))

from core.agentic_tools import AgenticTools, execute_tool
from core.agentic_strategies import get_strategy
from core.trajectory_builder import TrajectoryBuilder
# from core.similarity import compute_code_cosine_similarity, extract_ground_truth_by_id
from orm_scorer import load_orm_scorer


def log_error(error_log_file, error_type, details):
    """Log errors to dedicated error log file"""
    import datetime
    import traceback
    
    with open(error_log_file, 'a') as f:
        f.write("\n" + "="*80 + "\n")
        f.write(f"ERROR: {error_type}\n")
        f.write(f"TIMESTAMP: {datetime.datetime.now().isoformat()}\n")
        f.write("-"*80 + "\n")
        f.write(json.dumps(details, indent=2, default=str))
        f.write("\n")
        f.write(f"TRACEBACK:\n{traceback.format_exc()}")
        f.write("\n" + "="*80 + "\n")


def call_api_with_tools(conversation, tools, model, api_key, cooldown=1.5, max_tokens=4000, error_log_file=None, max_retries=3, temperature=0.0):
    """
    Call inference API with tool support
    Uses Bearer token authentication
    cooldown: seconds to wait between calls (default 1.5s for 40 rpm limit)
    max_tokens: per-turn token limit (default 4000)
    error_log_file: path to error log file for detailed error logging
    max_retries: number of retries for timeout errors (default 3)
    temperature: sampling temperature (0.0=greedy, 0.7=diverse)
    """
    
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/json"
    }
    
    # Validate conversation structure for Mistral models
    if 'mistral' in model.lower():
        # Ensure no consecutive tool messages without assistant response
        cleaned_conversation = []
        for i, msg in enumerate(conversation):
            if msg.get('role') == 'tool':
                # Check if previous message was also a tool message
                if cleaned_conversation and cleaned_conversation[-1].get('role') == 'tool':
                    # Insert empty assistant message
                    cleaned_conversation.append({"role": "assistant", "content": ""})
            cleaned_conversation.append(msg)
        conversation = cleaned_conversation
    
    payload = {
        "model": model,
        "messages": conversation,
        "temperature": temperature,
        "max_tokens": max_tokens,
        "stream": False
    }
    
    if tools:
        payload["tools"] = tools
        payload["tool_choice"] = "auto"
    
    # Validate JSON serialization before sending
    try:
        json.dumps(payload)
    except (TypeError, ValueError) as e:
        if error_log_file:
            log_error(error_log_file, "JSON_SERIALIZATION_ERROR", {
                "error": str(e),
                "conversation_length": len(conversation),
                "last_messages": conversation[-3:] if len(conversation) >= 3 else conversation
            })
        print(f"❌ JSON serialization error: {e}")
        return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "json_error", "error_message": str(e)}
    
    import time
    
    # Retry loop for timeout errors
    for attempt in range(max_retries):
        try:
            request_start = time.time()
            
            # Increase timeout progressively: 300s, 450s, 600s
            timeout = 300 * (1 + attempt * 0.5)
            
            if attempt > 0:
                print(f"  🔄 Retry {attempt}/{max_retries-1} with {timeout:.0f}s timeout...")
            
            response = requests.post(
                "{inference_provider}",
                headers=headers,
                json=payload,
                timeout=timeout
            )
            
            request_duration = time.time() - request_start
        
            # Log only errors to error log (not 200 responses)
            if error_log_file and response.status_code != 200:
                with open(error_log_file, 'a') as f:
                    f.write(f"\n[API REQUEST ERROR] Duration: {request_duration:.2f}s, Status: {response.status_code}\n")
            
            # Handle rate limiting with retry
            if response.status_code == 429:
                retry_after = int(response.headers.get('Retry-After', 60))
                print(f"⏸️  Rate limit hit, waiting {retry_after}s...")
                
                if error_log_file:
                    log_error(error_log_file, "RATE_LIMIT", {
                        "status_code": 429,
                        "retry_after": retry_after,
                        "headers": dict(response.headers),
                        "response": response.text[:1000]
                    })
                
                time.sleep(retry_after)
                response = requests.post(
                    "{inference_provider}",
                    headers=headers,
                    json=payload,
                    timeout=timeout
                )
            
            if response.status_code != 200:
                error_msg = f"❌ API Error {response.status_code}: {response.text[:500]}"
                print(error_msg)
                
                if error_log_file:
                    # Log the FULL payload for 400 errors (Bad Request)
                    payload_to_log = payload.copy()
                    if response.status_code == 400:
                        # Include full conversation for debugging
                        payload_to_log['messages'] = conversation
                    else:
                        # For other errors, just log metadata
                        payload_to_log['messages'] = f"[{len(conversation)} messages]"
                    
                    log_error(error_log_file, "API_ERROR", {
                        "status_code": response.status_code,
                        "headers": dict(response.headers),
                        "response_text": response.text,
                        "request_payload": payload_to_log,
                        "conversation_length": len(conversation)
                    })
                
                # Retry on 5xx server errors
                if response.status_code >= 500 and attempt < max_retries - 1:
                    backoff = 15 * (2 ** attempt)
                    print(f"  🔄 Server error {response.status_code}, retrying in {backoff}s...")
                    time.sleep(backoff)
                    continue
            
            response.raise_for_status()
            
            result = response.json()
            
            # Safety check for API response structure
            if 'choices' not in result or not result['choices']:
                error_msg = f"Invalid API response: no choices returned"
                print(f"❌ {error_msg}")
                if error_log_file:
                    log_error(error_log_file, "INVALID_RESPONSE", {
                        "result": result,
                        "conversation_length": len(conversation)
                    })
                return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}}
            
            message = result['choices'][0]['message']
            
            # Add usage info
            message['usage'] = result.get('usage', {'total_tokens': 500})
            
            # Cooldown to respect rate limits
            time.sleep(cooldown)
            
            return message
        
        except requests.exceptions.Timeout as e:
            # Last attempt - give up
            if attempt == max_retries - 1:
                error_msg = f"API request timeout after {max_retries} attempts (max {timeout:.0f}s): {e}"
                print(f"❌ {error_msg}")
                if error_log_file:
                    log_error(error_log_file, "TIMEOUT_FINAL", {
                        "error": str(e),
                        "timeout": timeout,
                        "attempts": max_retries,
                        "conversation_length": len(conversation)
                    })
                return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "timeout", "error_message": str(e)}

            # Retry with exponential backoff
            backoff = 10 * (2 ** attempt)
            print(f"⏰ Timeout after {timeout:.0f}s, waiting {backoff}s before retry...")
            if error_log_file:
                log_error(error_log_file, f"TIMEOUT_RETRY_{attempt+1}", {
                    "error": str(e),
                    "timeout": timeout,
                    "attempt": attempt + 1,
                    "backoff": backoff,
                    "conversation_length": len(conversation)
                })
            time.sleep(backoff)
            continue
        
        except requests.exceptions.ConnectionError as e:
            error_msg = f"Connection error: {e}"
            print(f"❌ {error_msg}")
            if error_log_file:
                log_error(error_log_file, "CONNECTION_ERROR", {
                    "error": str(e),
                    "url": "{inference_provider}"
                })
            if attempt < max_retries - 1:
                backoff = 15 * (2 ** attempt)
                print(f"  🔄 Connection error, retrying in {backoff}s...")
                time.sleep(backoff)
                continue
            return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "connection_error", "error_message": str(e)}
        
        except Exception as e:
            error_msg = f"Error calling inference API: {e}"
            print(f"❌ {error_msg}")
            if error_log_file:
                log_error(error_log_file, "UNKNOWN_ERROR", {
                    "error": str(e),
                    "error_type": type(e).__name__,
                    "conversation_length": len(conversation)
                })
            return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "unknown_error", "error_message": str(e)}
    
    # Should never reach here
    return {"content": "", "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "unknown_error", "error_message": "Unknown error in API call"}


def call_api_streaming(conversation, tools, model, api_key, cooldown=1.5,
                              max_tokens=4000, error_log_file=None, max_retries=3,
                              temperature=0.6, top_p=0.7):
    """Call inference API with streaming for thinking models (reasoning_content).
    
    Thinking models (e.g. qwen3-next-80b-a3b-thinking) require streaming=True
    to capture reasoning_content alongside tool_calls.
    Returns same format as call_api_with_tools but with added
    'reasoning_content' field.
    """
    import time
    
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/json"
    }
    
    payload = {
        "model": model,
        "messages": conversation,
        "temperature": temperature,
        "top_p": top_p,
        "max_tokens": max_tokens,
        "stream": True
    }
    
    if tools:
        payload["tools"] = tools
        payload["tool_choice"] = "auto"
    
    # Add model-specific thinking params (thinking_budget, chat_template_kwargs, etc.)
    thinking_params = _get_thinking_params(model)
    for key, val in thinking_params.items():
        payload[key] = val
    
    for attempt in range(max_retries):
        try:
            request_start = time.time()
            timeout = 300 * (1 + attempt * 0.5)
            
            if attempt > 0:
                print(f"  🔄 Retry {attempt}/{max_retries-1} (streaming)...")
            
            resp = requests.post(
                "{inference_provider}",
                headers=headers,
                json=payload,
                timeout=timeout,
                stream=True
            )
            
            if resp.status_code == 429:
                retry_after = int(resp.headers.get('Retry-After', 60))
                print(f"⏸️  Rate limit hit, waiting {retry_after}s...")
                if error_log_file:
                    log_error(error_log_file, "RATE_LIMIT_STREAM", {
                        "status_code": 429, "retry_after": retry_after
                    })
                time.sleep(retry_after)
                continue
            
            if resp.status_code != 200:
                error_msg = f"❌ Streaming API Error {resp.status_code}: {resp.text[:500]}"
                print(error_msg)
                if error_log_file:
                    log_error(error_log_file, "API_ERROR_STREAM", {
                        "status_code": resp.status_code,
                        "response_text": resp.text[:2000]
                    })
                resp.raise_for_status()
            
            # Accumulate streamed chunks
            reasoning_parts = []
            content_parts = []
            tool_calls_data = {}
            usage_data = {}
            
            for line in resp.iter_lines():
                line_str = line.decode('utf-8')
                if not line_str.startswith('data: '):
                    continue
                data = line_str[6:]
                if data == '[DONE]':
                    break
                try:
                    chunk = json.loads(data)
                    
                    # Capture usage from final chunk
                    if 'usage' in chunk:
                        usage_data = chunk['usage']
                    
                    if not chunk.get('choices'):
                        continue
                    delta = chunk['choices'][0].get('delta', {})
                    
                    # Reasoning content (thinking model's chain-of-thought)
                    rc = delta.get('reasoning_content')
                    if rc:
                        reasoning_parts.append(rc)
                    
                    # Regular content
                    c = delta.get('content')
                    if c:
                        content_parts.append(c)
                    
                    # Tool calls (accumulated across chunks)
                    tcs = delta.get('tool_calls')
                    if tcs:
                        for tc in tcs:
                            idx = tc.get('index', 0)
                            if idx not in tool_calls_data:
                                tool_calls_data[idx] = {
                                    'id': '', 'type': 'function',
                                    'function': {'name': '', 'arguments': ''}
                                }
                            if tc.get('id'):
                                tool_calls_data[idx]['id'] = tc['id']
                            fn = tc.get('function', {})
                            if fn.get('name'):
                                tool_calls_data[idx]['function']['name'] += fn['name']
                            if fn.get('arguments'):
                                tool_calls_data[idx]['function']['arguments'] += fn['arguments']
                except (json.JSONDecodeError, KeyError, IndexError):
                    pass
            
            request_duration = time.time() - request_start
            
            # Build response in same format as non-streaming
            reasoning = ''.join(reasoning_parts)
            content = ''.join(content_parts)
            tool_calls_list = None
            if tool_calls_data:
                tool_calls_list = [tool_calls_data[k] for k in sorted(tool_calls_data.keys())]
            
            # Estimate tokens if usage not provided in stream
            if not usage_data:
                # Rough estimate: 1 token ≈ 4 chars
                est_completion = (len(reasoning) + len(content)) // 4
                usage_data = {'total_tokens': est_completion + 500, 'completion_tokens': est_completion}
            
            message = {
                'role': 'assistant',
                'content': content if content else None,
                'reasoning_content': reasoning if reasoning else None,
                'tool_calls': tool_calls_list,
                'usage': usage_data
            }
            
            reasoning_preview = reasoning[:100] + '...' if len(reasoning) > 100 else reasoning
            if reasoning:
                print(f"  💭 Thinking: {reasoning_preview}")
            
            time.sleep(cooldown)
            return message
        
        except requests.exceptions.Timeout as e:
            if attempt == max_retries - 1:
                print(f"❌ Streaming timeout after {max_retries} attempts: {e}")
                if error_log_file:
                    log_error(error_log_file, "TIMEOUT_STREAM", {"error": str(e)})
                return {"content": "", "reasoning_content": None, "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "timeout", "error_message": str(e)}
            backoff = 10 * (2 ** attempt)
            print(f"⏰ Timeout, waiting {backoff}s before retry...")
            time.sleep(backoff)
            continue
        
        except Exception as e:
            print(f"❌ Streaming error: {e}")
            if error_log_file:
                log_error(error_log_file, "STREAM_ERROR", {"error": str(e), "type": type(e).__name__})
            return {"content": "", "reasoning_content": None, "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "unknown_error", "error_message": str(e)}
    
    return {"content": "", "reasoning_content": None, "tool_calls": None, "usage": {"total_tokens": 0}, "status_code": "unknown_error", "error_message": "Unknown error in streaming API call"}


class CppExtractor:
    """AST-centric C++ code extractor with JSON/markdown fallback.
    
    Extraction pipeline:
    1. JSON parsing (extract 'function_body' key)
    2. Markdown/XML wrapper stripping (deterministic string scanners)
    3. AST validation and ranking (tree-sitter)
    4. Signature stripping (regex fallback for malformed code)
    5. Final function body extraction (AST or cleaned text)
    """
    
    def __init__(self):
        from tree_sitter import Parser, Language
        import tree_sitter_cpp as tscpp
        self.parser = Parser(Language(tscpp.language()))
    
    # ── AST helpers ──────────────────────────────────────────────────────
    
    @staticmethod
    def _is_json(text: str) -> bool:
        """Return True if *text* is valid JSON (object or array)."""
        import json
        text = text.strip()
        if not text or text[0] not in ('{', '['):
            return False
        try:
            json.loads(text)
            return True
        except (json.JSONDecodeError, ValueError):
            return False

    @staticmethod
    def _has_cpp_indicators(code: str) -> bool:
        """Quick heuristic: does *code* contain at least one C++ indicator?
        
        Checks for keywords, operators, or syntax that only appear in C/C++.
        Returns False for JSON, plain English, etc.
        """
        import re
        # C++ keywords (word-boundary safe)
        CPP_KEYWORDS = re.compile(
            r'\b(?:return|if|else|for|while|switch|case|break|continue|'
            r'void|int|char|float|double|bool|auto|const|static|'
            r'class|struct|enum|namespace|template|typename|'
            r'nullptr|new|delete|throw|try|catch|sizeof)\b'
        )
        # C++ operators / syntax (no word boundary needed)
        CPP_SYNTAX = re.compile(
            r'(?:std::|::|->|<<|>>|#include|#define|#ifdef|#ifndef|#pragma)'
        )
        return bool(CPP_KEYWORDS.search(code) or CPP_SYNTAX.search(code))

    def _ast_quality(self, code: str) -> float:
        """Score code via tree-sitter: ratio of non-error AST nodes.
        
        Returns 0.0 for empty/unparseable, 1.0 for perfectly clean AST.
        Also returns 0.0 for content that is valid JSON (not C++).
        Used to rank competing extraction candidates.
        """
        code = code.strip()
        if not code:
            return 0.0
        # JSON is never valid C++ — reject regardless of tree-sitter score
        if self._is_json(code):
            return 0.0
        tree = self.parser.parse(bytes(code, "utf8"))
        total = 0
        errors = 0
        stack = [tree.root_node]
        while stack:
            node = stack.pop()
            total += 1
            if node.type == "ERROR" or node.is_missing:
                errors += 1
            stack.extend(node.children)
        return 1.0 - (errors / max(total, 1))
    
    def _find_function_body(self, code: str):
        """Walk tree-sitter AST looking for function_definition → body.
        
        Returns the body text (braces stripped) or None.
        """
        import re
        tree = self.parser.parse(bytes(code, "utf8"))
        
        def walk(node):
            if node.type == "function_definition":
                body = node.child_by_field_name("body")
                if body:
                    body_text = code[body.start_byte:body.end_byte]
                    if body_text.startswith("{") and body_text.endswith("}"):
                        body_text = body_text[1:-1].strip()
                    body_text = re.sub(r'^\s*//\s*FUNCTION_ID:.*?(?:START|END)\s*$',
                                       '', body_text, flags=re.MULTILINE)
                    body_text = re.sub(r'\n\n\n+', '\n\n', body_text).strip()
                    return body_text
            for child in node.children:
                result = walk(child)
                if result is not None:
                    return result
            return None
        
        try:
            return walk(tree.root_node)
        except Exception:
            return None
    
    # ── Wrapper strippers (deterministic, no regex for delimiters) ───────
    
    def _strip_json(self, content: str):
        """Extract code from JSON {"function_body": "..."}.
        
        Handles:
        - Complete JSON: {"function_body": "code"}
        - Embedded JSON: text before {"function_body": "code"} text after
        - Partial JSON: {"function_body": "code" (missing closing brace)
        """
        import json
        
        # Try complete JSON first
        if content.strip().startswith("{") and content.strip().endswith("}"):
            try:
                data = json.loads(content.strip())
                body = data.get("function_body")
                if body:
                    return body
            except (json.JSONDecodeError, ValueError):
                pass
        
        # Try to find JSON within the response
        start = content.find("{")
        end = content.rfind("}") + 1
        if start != -1 and end > start:
            try:
                json_part = content[start:end]
                data = json.loads(json_part)
                body = data.get("function_body")
                if body:
                    return body
            except (json.JSONDecodeError, ValueError):
                pass
        
        return None
    
    def _strip_tool_call(self, content: str):
        """Parse <tool_call> XML using str.find() scanner (not regex).
        
        Handles:  <tool_call><function=...><parameter=code>CODE</parameter>...</tool_call>
        
        Uses exact string matching for delimiters — O(n) scan, immune to
        C++ angle brackets confusing a regex engine.
        If the closing tag is missing (malformed response), falls back to
        extracting everything after the open tag and validating with AST.
        """
        OPEN_TAG = "<parameter=code>"
        CLOSE_TAG = "</parameter>"
        
        open_idx = content.find(OPEN_TAG)
        if open_idx == -1:
            return None
        
        code_start = open_idx + len(OPEN_TAG)
        close_idx = content.find(CLOSE_TAG, code_start)
        
        if close_idx != -1:
            extracted = content[code_start:close_idx].strip()
            if extracted:
                return extracted
        else:
            # Malformed: no closing </parameter>. Salvage by taking everything
            # after the open tag and only returning if AST validates it.
            candidate = content[code_start:].strip()
            # Strip trailing XML tags that might remain
            for trailing in ("</function>", "</tool_call>"):
                pos = candidate.rfind(trailing)
                if pos != -1:
                    candidate = candidate[:pos].strip()
            if candidate and self._ast_quality(candidate) > 0.3:
                return candidate
        
        return None
    
    def _strip_markdown(self, content: str):
        """Extract code from markdown fences using str.find() scanner.
        
        Handles:
        - Standard: ```cpp\ncode\n```
        - Generic: ```\ncode\n```
        - Truncated: ```cpp\ncode (no closing fence)
        """
        # Find opening fence: ``` possibly followed by a language tag
        fence_start = content.find("```")
        if fence_start == -1:
            return None
        
        # Skip the language tag (everything until newline after ```)
        newline_after = content.find("\n", fence_start)
        if newline_after == -1:
            # No newline after ``` - malformed, but try to extract anyway
            code_start = fence_start + 3
            # Skip language tag if present (e.g., ```cpp)
            while code_start < len(content) and content[code_start].isalpha():
                code_start += 1
        else:
            code_start = newline_after + 1
        
        # Find closing fence
        fence_end = content.find("```", code_start)
        if fence_end != -1:
            extracted = content[code_start:fence_end].strip()
        else:
            # Truncated - no closing ```, take everything after opening
            extracted = content[code_start:].strip()
        
        return extracted if extracted else None
    
    # ── Main pipeline ────────────────────────────────────────────────────
    
    def extract_code_block(self, content: str) -> str:
        """AST-ranked code block extraction.
        
        Tries multiple strippers, scores each candidate via tree-sitter AST,
        and returns the highest-quality one. No regex is used for delimiter
        matching — only tree-sitter for C++ validation and str.find() for
        wrapper boundaries.
        
        Pipeline:
        1. Collect candidates from JSON / tool_call XML / markdown strippers
        2. Score each with _ast_quality()
        3. Return best candidate (highest AST score)
        4. If no candidates, return raw content
        """
        content = content.strip()
        if not content:
            return ""
        
        # Collect (candidate, source) pairs
        candidates = []
        
        json_code = self._strip_json(content)
        if json_code:
            candidates.append(("json", json_code))
        
        xml_code = self._strip_tool_call(content)
        if xml_code:
            candidates.append(("tool_call", xml_code))
        
        md_code = self._strip_markdown(content)
        if md_code:
            candidates.append(("markdown", md_code))
        
        if not candidates:
            # Guard: never return raw JSON as C++ code (e.g. tool-call args)
            if self._is_json(content):
                return ""
            # Guard: if no C++ indicators at all, reject
            if not self._has_cpp_indicators(content):
                return ""
            return content
        
        # Single candidate — skip scoring overhead
        if len(candidates) == 1:
            return candidates[0][1]
        
        # Multiple candidates — let AST decide
        best_code = None
        best_score = -1.0
        for _source, code in candidates:
            score = self._ast_quality(code)
            if score > best_score:
                best_score = score
                best_code = code
        
        return best_code if best_code else content
    
    def extract_first_function_body(self, code: str) -> str:
        """Extract first function_definition body using AST.
        
        Falls back to signature stripping if no function_definition is found.
        Returns empty string if content is JSON or has no C++ indicators.
        """
        import re
        
        body = self._find_function_body(code)
        if body is not None:
            return body
        
        # No function_definition found — validate before returning raw code
        # Guard: reject JSON that leaked through a wrapper stripper,
        # but first check if it has a function_body key we can extract
        if self._is_json(code):
            json_body = self._strip_json(code)
            return json_body if json_body else ""
        # Guard: reject content with no C++ indicators
        if not self._has_cpp_indicators(code):
            return ""
        
        # Try signature stripping as fallback (for malformed code)
        cleaned = self._strip_function_signature(code)
        
        try:
            # Remove FUNCTION_ID markers
            cleaned = re.sub(r'^\s*//\s*FUNCTION_ID:.*?(?:START|END)\s*$',
                             '', cleaned, flags=re.MULTILINE)
            cleaned = re.sub(r'\n\n\n+', '\n\n', cleaned).strip()
            return cleaned
        except Exception:
            return cleaned.strip()
    
    def _strip_function_signature(self, code: str) -> str:
        """Strip function signature if present using regex.
        
        Handles:
        - Template functions: template<typename T> void func() { body }
        - Virtual/static/inline: virtual void func() { body }
        - Const/override/noexcept: void func() const override { body }
        - Pointers/references: int* func() { body }
        - Namespaces: MyClass::func() { body }
        
        Returns:
        - Function body without signature and outer braces
        - Original code if no signature pattern found
        """
        import re
        
        cleaned = code.strip()
        
        # Pattern matches function signature with opening brace
        signature_pattern = r'^\s*(?:template\s*<[^>]+>\s*)?(?:virtual\s+)?(?:static\s+)?(?:inline\s+)?[\w:]+(?:\s*[\*&])?\s+[\w:~]+\s*\([^)]*\)\s*(?:const)?\s*(?:override)?\s*(?:noexcept)?\s*\{'
        match = re.match(signature_pattern, cleaned, re.DOTALL)
        
        if match:
            # Remove signature and opening brace
            cleaned = cleaned[match.end():]
            # Remove closing brace at end
            cleaned = re.sub(r'\}\s*$', '', cleaned).strip()
        
        return cleaned
    
    def extract(self, content: str) -> str:
        """Main extraction: strip wrappers → AST function body."""
        code = self.extract_code_block(content)
        return self.extract_first_function_body(code)


# Global extractor instance (reusable)
_cpp_extractor = None

def extract_code(content):
    """Extract C++ code from response - body only, no signature (Tree-sitter based)"""
    global _cpp_extractor
    if _cpp_extractor is None:
        _cpp_extractor = CppExtractor()
    return _cpp_extractor.extract(content)


def ast_quality_score(code: str) -> float:
    """Score C++ code via tree-sitter AST (0.0–1.0, ratio of non-error nodes).
    
    Useful as a PRM signal: higher = more syntactically valid.
    """
    global _cpp_extractor
    if _cpp_extractor is None:
        _cpp_extractor = CppExtractor()
    return _cpp_extractor._ast_quality(code)


def compress_conversation(conversation, keep_last_n=5):
    """Keep system prompt + last N turns + extract key facts from old turns"""
    if len(conversation) <= keep_last_n + 1:
        return conversation
    
    system_msg = conversation[0]
    old_turns = conversation[1:-keep_last_n]
    recent_turns = conversation[-keep_last_n:]
    
    # Extract key information without API call
    tools_used = []
    files_read = []
    key_findings = []
    
    for msg in old_turns:
        # Extract tool calls
        if msg.get('role') == 'assistant' and msg.get('tool_calls'):
            for tc in msg['tool_calls']:
                tool_name = tc['function']['name']
                tools_used.append(tool_name)
                
                # Extract file names from read_files
                if tool_name == 'read_files':
                    try:
                        args = json.loads(tc['function']['arguments'])
                        files_read.extend(args.get('file_paths', []))
                    except:
                        pass
        
        # Extract tool results with key info
        if msg.get('role') == 'tool':
            content = msg.get('content', '')
            if len(content) > 100:
                key_findings.append(content[:100] + '...')
    
    # Build intelligent summary
    summary_parts = [f"[Previous exploration: {len(old_turns)} messages]"]
    
    if tools_used:
        tool_counts = {}
        for t in tools_used:
            tool_counts[t] = tool_counts.get(t, 0) + 1
        summary_parts.append(f"Tools used: {', '.join(f'{k}({v}x)' for k, v in tool_counts.items())}")
    
    if files_read:
        unique_files = list(set(files_read))[:3]
        summary_parts.append(f"Files examined: {', '.join(unique_files)}")
    
    summary = ' | '.join(summary_parts)
    
    return [system_msg, {"role": "user", "content": summary}] + recent_turns


def compress_conversation_with_llm(conversation, keep_last_n, model, api_key):
    """OPTIONAL: Use LLM to summarize (costs extra tokens but higher quality)"""
    if len(conversation) <= keep_last_n + 1:
        return conversation
    
    system_msg = conversation[0]
    old_turns = conversation[1:-keep_last_n]
    recent_turns = conversation[-keep_last_n:]
    
    # Build summarization prompt
    turns_text = "\n".join([
        f"{msg.get('role', 'unknown')}: {str(msg.get('content', ''))[:200]}"
        for msg in old_turns
    ])
    
    summary_prompt = [
        {"role": "system", "content": "Summarize the conversation in 2-3 sentences focusing on: tools used, files examined, key findings."},
        {"role": "user", "content": f"Summarize this:\n{turns_text}"}
    ]
    
    try:
        response = call_api_with_tools(
            conversation=summary_prompt,
            tools=None,
            model=model,
            api_key=api_key,
            cooldown=0,
            max_tokens=150
        )
        summary = response.get('content', '[Summary unavailable]')
    except:
        summary = f"[Previous {len(old_turns)} messages summarized]"
    
    return [system_msg, {"role": "user", "content": summary}] + recent_turns


def _extract_think_reasoning(content: str) -> str:
    """Extract reasoning from <think>...</think> tags in LLM response.
    
    Falls back to full content if no tags found.
    Returns empty string only if content is truly empty.
    """
    if not content:
        return ''
    
    import re
    # Extract all <think> blocks (there may be multiple)
    thinks = re.findall(r'<think>(.*?)</think>', content, re.DOTALL)
    if thinks:
        return '\n'.join(t.strip() for t in thinks)
    
    # No <think> tags — use full content as reasoning (model didn't follow format)
    # But strip any code blocks to get just the prose
    stripped = re.sub(r'```[\s\S]*?```', '', content).strip()
    return stripped if stripped else content.strip()


# ── Thinking model registry ─────────────────────────────────────
# Models that produce reasoning_content via streaming.
# Each entry: model_substring → extra payload keys for streaming call.

_THINKING_MODELS = {
    'thinking':       {},                                                    # qwen3-next-80b-a3b-thinking
    'gpt-oss':        {},                                                    # openai/gpt-oss-120b
    'seed-oss':       {'extra_body': {'thinking_budget': -1}},               # bytedance/seed-oss-36b-instruct
    'qwen3.5-397b':   {'chat_template_kwargs': {'enable_thinking': True}},   # qwen/qwen3.5-397b-a17b
}

def _is_thinking_model(model: str) -> bool:
    """Check if a model supports reasoning_content via streaming."""
    model_lower = model.lower()
    return any(key in model_lower for key in _THINKING_MODELS)

def _get_thinking_params(model: str) -> dict:
    """Get model-specific extra payload params for thinking mode."""
    model_lower = model.lower()
    for key, params in _THINKING_MODELS.items():
        if key in model_lower:
            return params
    return {}


def _reconstruct_reasoning_from_tool(tool_name: str, tool_args: dict, step: int) -> str:
    """Reconstruct minimal reasoning when model provides no <think> tags.
    
    This is a fallback — PRM training strongly prefers real model reasoning.
    Marked with [auto-reconstructed] so PRM pipeline can filter/downweight.
    """
    descriptions = {
        'search_files': lambda a: f"Searching for files matching '{a.get('query', '?')}' to locate relevant source code.",
        'search_repo_code': lambda a: f"Searching codebase for pattern '{a.get('query', '?')}' to understand usage/implementation.",
        'read_files': lambda a: f"Reading {a.get('paths', ['?'])} to examine code structure and API patterns.",
        'list_directory': lambda a: f"Listing directory '{a.get('path', '.')}' to understand project layout.",
        'test_code_snippet': lambda a: f"Testing code compilation in Docker to validate implementation correctness.",
    }
    
    desc_fn = descriptions.get(tool_name)
    desc = desc_fn(tool_args) if desc_fn else f"Calling {tool_name} with {list(tool_args.keys())}."
    return f"[auto-reconstructed] Step {step}: {desc}"


def solve_agentic_task(task, namespace, signature, tools, strategy_name,
                              strategy_config, model, api_key, force_answer, log_file, api_cooldown=1.5,
                              repo_name="", task_id="", save_trajectory=True, temperature=0.0,
                              enable_thinking=False):
    """Solve task using inference API"""
    
    # Create error log file
    error_log_file = str(log_file).replace('.txt', '_errors.log')
    
    # Initialize strategy
    strategy = get_strategy(strategy_name, strategy_config)
    strategy.start()
    
    # Initialize trajectory builder
    trajectory = TrajectoryBuilder(
        task_id=task_id or namespace,
        repo=repo_name,
        model=model,
        budget=strategy_config,
        beam_id=1,  # Will be overridden when saving with actual beam_id
        temperature=temperature
    ) if save_trajectory else None
    
    # System prompt with persona
    system_prompt = f"""You are Dr. CodeMaster, a senior C++ architect with 15+ years of experience in production systems. You excel at understanding complex codebases quickly and writing clean, efficient implementations.

    === YOUR MISSION ===
{task}

Function signature: {signature}

=== TASK CONTEXT ===
Namespace: {namespace}
You need to implement this function by exploring the repository and understanding the codebase patterns.

=== YOUR RESOURCES ===

COMPLETE STANDARD C++ LIBRARY (all pre-included):
- Containers: vector, map, set, unordered_map, queue, stack, deque, list etc.
- Algorithms: sort, find, binary_search, lower_bound, upper_bound, max, min etc.
- Math: sqrt, pow, abs, floor, ceil, sin, cos etc.
- Utilities: string, pair, tuple, unique_ptr, shared_ptr etc.

AVAILABLE TOOLS:
- search_repo_code: Search codebase for patterns
- read_files: Read files to understand structure
- test_code_snippet: **CRITICAL** Test if code compiles in Docker - USE THIS BEFORE SUBMITTING!
- list_directory: Browse directory structure
- search_files: Find files by name

⚠️ IMPORTANT: Working directory is 'context/'. All paths are relative to context/.
Example: search_files returns 'adhesion/edge_count.cc' → use that exact path in read_files
DO NOT use absolute paths like '/adhesion/...' - they will fail! 

⚠️ TOKEN MANAGEMENT - CRITICAL:
    - Your per-turn limit is {strategy_config.get('max_tokens_per_turn', 7000)} tokens
- Total budget: {strategy_config['max_tokens']} tokens for entire task
- Tools return truncated data to save tokens
- Use pagination parameters to control data:
  * search_repo_code(query, max_results=5, start_index=0)
  * read_files(paths, max_chars=5000, start_char=0)
  * search_files(query, max_results=20, start_index=0)
- Read large files in chunks: start_char=0, then 10000, then 20000...
- Request fewer results first, get more only if needed
- Don't overwhelm your context - be strategic!

=== YOUR BUDGET ===
- {strategy_config['max_iterations']} iterations
- {strategy_config['max_tool_calls']} tool calls (use wisely!)
- {strategy_config['max_tokens']} tokens
- {strategy_config['max_time']} seconds
⚠️ TOOL USAGE STRATEGY:
- Exploration: Use 4-6 tool calls to understand key APIs and patterns
- Efficiency: Don't over-explore - focus on what you need


=== CRITICAL: SUBMISSION RULES ===

Follow modern C++ best practices:
- ALWAYS prefix standard library types/functions with std::
  Example: std::vector, std::map, std::swap, std::sqrt
- NEVER use `using namespace std`

When you provide code in a ```cpp block:
- It will be compiled and executed against hidden test cases
- If it compiles AND passes tests → task complete

=== CRITICAL: OUTPUT FORMAT ===

Provide ONLY the function body code inside ```cpp markers.
- DO NOT include the function signature
- Do NOT add #include statements. The file already contains all required headers.
- DO NOT include the opening/closing braces of the function itself
- Start directly with the implementation code
- If you include the signature, your answer will be REJECTED
- Also make sure that the function is self-sufficient and self-contained.

EXAMPLE - CORRECT:
```cpp
int count = 0;
for (int i = 0; i < n; i++) {{
    count++;
}}
return count;
```

EXAMPLE - WRONG (includes signature):
```cpp
int MyClass::count_items(int n) const {{  // ← DO NOT include this!
    int count = 0;
    for (int i = 0; i < n; i++) {{
        count++;
    }}
    return count;
}}  // ← DO NOT include function's closing brace
```

EXAMPLE - WRONG (includes function wrapper):
```cpp
void function_name(params) {{
    // code here
}}
```

The function signature already exists in the source file. You are completing the body only.
Provide ONLY the code that goes INSIDE the function braces.

Ensure the code compiles:
- All identifiers must exist in the repository
- Do not invent new APIs or types


=== CRITICAL: REASONING CHAIN ===

Before EVERY action (tool call or code submission), you MUST provide a clear, step-by-step explanation of your reasoning as plain text. If your model does not support <think> tags, simply write your reasoning as a paragraph before each tool call or code block. This is MANDATORY for every single response — never skip it.

Example of a complete reasoning chain across multiple turns (plain text, no <think> tags required):

Turn 1:
I need to implement a Delaunay triangulation function. First, I should understand the existing data structures. The class uses Triangulation2, so I will search for the header file to understand the API surface — vertex types, face handles, insertion methods. Plan: search headers → read key types → find similar functions → implement → test.
[calls search_files]

Turn 2:
Found triangulation2.h. Key types: Vertex_handle, Face_handle, and insert() takes Point_2. The class wraps CGAL's Delaunay_triangulation_2 internally. Now I need to read the source file to see how similar functions handle edge cases and what return type is expected.
[calls read_files]

Turn 3:
From reading the source, I see the pattern: create Point_2 from coords, call this->_triangulation.insert(p), then update the face count. The function should return the new vertex handle. I have enough context to implement now — let me write it and validate with test_code_snippet before submitting.
[calls test_code_snippet with implementation]

Remember: You're a master craftsman. Work efficiently, think clearly, and submit only when confident."""

    conversation = [
        {"role": "system", "content": system_prompt},
        {"role": "user", "content": f"Implement the function: {signature}"}
    ]
    trace = []
    rl_trajectory = []
    step_counter = 0
    compile_attempts = 0
    last_compile_errors = []
    
    # Log initial prompt
    with open(log_file, 'w') as f:
        import datetime
        f.write("="*80 + "\n")
        f.write(f"TASK: {namespace}\n")
        f.write(f"MODEL: {model}\n")
        f.write(f"STRATEGY: {strategy_name}\n")
        f.write(f"TIMESTAMP: {datetime.datetime.now().isoformat()}\n")
        f.write("="*80 + "\n\n")
        f.write("AGENT CONFIGURATION:\n")
        f.write("-"*80 + "\n")
        f.write(json.dumps(strategy_config, indent=2))
        f.write("\n" + "="*80 + "\n\n")
        
        # Log budget details
        f.write("BUDGET BREAKDOWN:\n")
        f.write("-"*80 + "\n")
        f.write(f"Total Token Budget: {strategy_config['max_tokens']:,} tokens\n")
        f.write(f"Per-Turn Token Limit: {strategy_config.get('max_tokens_per_turn', 7000):,} tokens\n")
        f.write(f"Max Iterations: {strategy_config['max_iterations']}\n")
        f.write(f"Max Tool Calls: {strategy_config['max_tool_calls']}\n")
        f.write(f"Max Time: {strategy_config['max_time']}s\n")
        f.write(f"Compression: {'DISABLED' if strategy_config.get('disable_compression', True) else 'ENABLED'}\n")
        f.write(f"\nEstimated tokens per iteration: {strategy_config['max_tokens'] // strategy_config['max_iterations']:,}\n")
        f.write(f"Tool calls per iteration: {strategy_config['tool_call_limit_per_iteration']}\n")
        f.write("\n" + "="*80 + "\n\n")
        
        f.write("INITIAL PROMPT:\n")
        f.write("-"*80 + "\n")
        f.write(json.dumps(conversation, indent=2))
        f.write("\n" + "="*80 + "\n\n")
    
    # Initialize error log
    with open(error_log_file, 'w') as f:
        import datetime
        f.write("="*80 + "\n")
        f.write(f"ERROR LOG: {namespace}\n")
        f.write(f"TIMESTAMP: {datetime.datetime.now().isoformat()}\n")
        f.write("="*80 + "\n")
    
    # Agent loop
    while not strategy.should_stop():
        strategy.next_iteration()
        print(f"\n🤖 Iteration {strategy.metrics['iterations']}")
        
        # Log iteration start with full conversation state
        with open(log_file, 'a') as f:
            f.write(f"\nITERATION {strategy.metrics['iterations']}:\n")
            f.write("-"*80 + "\n")
            f.write(f"Budget Status:\n")
            f.write(f"  Tokens: {strategy.metrics['tokens']:,}/{strategy_config['max_tokens']:,} ({strategy.metrics['tokens']/strategy_config['max_tokens']*100:.1f}%)\n")
            f.write(f"  Iterations: {strategy.metrics['iterations']}/{strategy_config['max_iterations']}\n")
            f.write(f"  Tool Calls: {strategy.metrics['tool_calls']}/{strategy_config['max_tool_calls']}\n")
            f.write(f"  Time: {strategy.metrics.get('elapsed_time', 0):.1f}s/{strategy_config['max_time']}s\n")
            f.write(f"  Conversation Length: {len(conversation)} messages\n")
            f.write("-"*80 + "\n")
            
            # Log full conversation state (for debugging/analysis)
            f.write(f"\nFULL CONVERSATION STATE ({len(conversation)} messages):\n")
            f.write("-"*80 + "\n")
            for i, msg in enumerate(conversation):
                # Persist the full message as structured JSON so logs contain
                # exact model inputs and tool-call arguments/results.
                try:
                    f.write(json.dumps({
                        'index': i + 1,
                        'role': msg.get('role', 'unknown'),
                        'content': msg.get('content', ''),
                        'tool_calls': msg.get('tool_calls', None),
                        'tool_call_id': msg.get('tool_call_id', None)
                    }, indent=2, default=str))
                    f.write('\n')
                except Exception:
                    # Fallback to a simple line if JSON serialization fails
                    role = msg.get('role', 'unknown')
                    content = msg.get('content', '')
                    f.write(f"\n[Message {i+1}] Role: {role}\n")
                    if content:
                        f.write(f"Content: {content}\n")
            
            f.write("\n" + "-"*80 + "\n")
        
        # Capture state (RL)
        state = {"iteration": strategy.metrics['iterations'], "tool_calls": strategy.metrics['tool_calls'], "tokens": strategy.metrics['tokens']}
        
        # Compress conversation if enabled and too long
        if not strategy_config.get('disable_compression', True) and len(conversation) > 12:
            # Use smart extraction (no API call, free)
            conversation = compress_conversation(conversation, keep_last_n=5)
            
            # OR use LLM summarization (costs tokens, higher quality)
            # conversation = compress_conversation_with_llm(
            #     conversation, keep_last_n=5, model=model, api_key=api_key
            # )
        
        # Calculate per-turn token budget
        remaining_tokens = strategy_config['max_tokens'] - strategy.metrics['tokens']
        per_turn_limit = strategy_config.get('max_tokens_per_turn', 7000)
        tokens_this_call = min(per_turn_limit, remaining_tokens, 10000)
        
        # Call inference API — use streaming for thinking models, non-streaming for instruct
        is_thinking_model = enable_thinking or _is_thinking_model(model)
        print(f"  ⏳ Calling API {'(streaming/thinking)' if is_thinking_model else '(non-streaming)'} (budget: {tokens_this_call:,} tokens, {remaining_tokens:,} remaining)...", flush=True)
        import time
        api_start_time = time.time()
        
        if is_thinking_model:
            response = call_api_streaming(
                conversation=conversation,
                tools=tools.get_tool_definitions(model_name=model),
                model=model,
                api_key=api_key,
                cooldown=api_cooldown,
                max_tokens=tokens_this_call,
                error_log_file=error_log_file,
                temperature=temperature,
                top_p=0.7
            )
        else:
            response = call_api_with_tools(
                conversation=conversation,
                tools=tools.get_tool_definitions(model_name=model),
                model=model,
                api_key=api_key,
                cooldown=api_cooldown,
                max_tokens=tokens_this_call,
                error_log_file=error_log_file,
                temperature=temperature
            )
        api_duration = time.time() - api_start_time
        # Set API status and error for trajectory (before any tool calls)
        api_status = None
        api_error = None
        api_response_code = response.get('status_code') or response.get('status')
        error_message = response.get('response_text') or response.get('error_message') or response.get('content', '')
        # If API call failed (status_code not 200 or is a string error), set error fields
        if api_response_code == 200:
            api_status = "ok"
            api_error = None
        elif api_response_code is not None:
            api_status = "error"
            api_error = f"❌ API Error {api_response_code}: {error_message[:500]}"
        elif response.get('error_message'):
            api_status = "error"
            api_error = f"❌ API Error: {response.get('error_message')[:500]}"

        # Patch: For forced_final_answer and non-tool-call steps, if result contains error info, set api_status/api_error accordingly
        # This ensures trajectory step reflects the error if present in result.full_result
        def patch_api_status_error_from_result(result):
            if not result:
                return None, None
            # Check for error info in result dict
            if isinstance(result, dict):
                full_result = result.get('full_result') if 'full_result' in result else result
                if isinstance(full_result, dict):
                    status_code = full_result.get('status_code')
                    error_message = full_result.get('error_message')
                    if status_code and status_code != 200:
                        return "error", f"❌ API Error {status_code}: {str(error_message)[:500]}"
                    elif error_message:
                        return "error", f"❌ API Error: {str(error_message)[:500]}"
            return None, None

        usage = response.get('usage', {})
        completion_tokens = usage.get('completion_tokens', 0)
        prompt_tokens = usage.get('prompt_tokens', 0)
        total_tokens = usage.get('total_tokens', 0)
        # Track only completion tokens for budget (prompt tokens are re-counted each turn)
        tokens_used = completion_tokens if completion_tokens else total_tokens
        print(f"  ✓ API responded in {api_duration:.1f}s (completion: {completion_tokens:,}, prompt: {prompt_tokens:,}, total: {total_tokens:,})")
        
        # Extract reasoning from response:
        # 1. Thinking models: use reasoning_content (native chain-of-thought)
        # 2. Instruct models: try <think> tags in content, else full content prose
        reasoning_content = response.get('reasoning_content', '') or ''
        raw_content = response.get('content', '') or ''
        
        if reasoning_content:
            # Thinking model — rich reasoning available
            reasoning_text = reasoning_content
        else:
            # Instruct model — try to extract from content
            reasoning_text = _extract_think_reasoning(raw_content)
        
        # Log API response with full details
        with open(log_file, 'a') as f:
            f.write(f"API CALL DETAILS:\n")
            f.write(f"  Duration: {api_duration:.2f}s\n")
            f.write(f"  Tokens Used: {tokens_used:,}\n")
            f.write(f"  Tokens Requested: {tokens_this_call:,}\n")
            f.write(f"  Cumulative Tokens: {strategy.metrics['tokens'] + tokens_used:,}\n")
            f.write(f"  Model Mode: {'thinking (streaming)' if is_thinking_model else 'instruct (non-streaming)'}\n")
            if reasoning_content:
                f.write(f"\nREASONING CONTENT ({len(reasoning_content)} chars):\n")
                f.write(reasoning_content)
                f.write("\n")
            f.write(f"\nAPI RESPONSE:\n")
            f.write(json.dumps(response, indent=2, default=str))
            f.write("\n\n")
        
        if response.get('content'):
            content_preview = response['content'][:500] if len(response.get('content', '')) > 500 else response['content']
            print(f"💭 Agent: {content_preview}")
            if len(response.get('content', '')) > 500:
                print(f"  ... (truncated, see log for full response)")
        
        # Track tokens (completion only — prompt tokens are re-counted each turn)
        strategy.add_tokens(tokens_used)
        
        # Check if 70% budget exhausted - send warning
        budget_pct = {
            'iterations': strategy.metrics['iterations'] / strategy_config['max_iterations'],
            'tool_calls': strategy.metrics['tool_calls'] / strategy_config['max_tool_calls'],
            'tokens': strategy.metrics['tokens'] / strategy_config['max_tokens']
        }
        max_budget_pct = max(budget_pct.values())
        
        if max_budget_pct >= 0.7 and not hasattr(strategy, '_warned_70'):
            strategy._warned_70 = True
            remaining_iterations = strategy_config['max_iterations'] - strategy.metrics['iterations']
            remaining_tools = strategy_config['max_tool_calls'] - strategy.metrics['tool_calls']
            remaining_tokens = strategy_config['max_tokens'] - strategy.metrics['tokens']
            

            warning_msg = (
                f"WARNING: BUDGET ALERT: You have used {max_budget_pct*100:.0f}% of your resources!\n"
                f"\nREMAINING BUDGET:\n"
                f"- {remaining_iterations} iterations\n"
                f"- {remaining_tools} tool calls\n"
                f"- {remaining_tokens} tokens\n"
                f"\nDr. CodeMaster, time to focus! Prioritize:\n"
                f"1. If you have enough context, generate the solution NOW\n"
                f"2. If not, make ONE final strategic tool call\n"
                f"3. Synthesize your findings and deliver the code\n"
                f"\nDeliver ONLY the C++ code block - no explanation. You're running out of time!"
            )
            
            print(f"\nWARNING: 70% budget threshold reached!")
            # Check last message role - if 'tool', add empty assistant message first (mistral models only)
            if 'mistral' in model.lower() and conversation and conversation[-1].get('role') == 'tool':
                conversation.append({"role": "assistant", "content": "Understood."})
            conversation.append({
                "role": "user",
                "content": warning_msg
            })
            
            with open(log_file, 'a') as f:
                f.write("\n" + "="*80 + "\n")
                f.write("70% BUDGET WARNING SENT\n")
                f.write("="*80 + "\n")
                f.write(warning_msg + "\n\n")
        
        # Handle tool calls - FIX 1: Capture ALL tool calls in ONE trajectory step
        if response.get('tool_calls'):
            num_tool_calls = len(response['tool_calls'])
            
            # Capture state BEFORE any tool calls are counted
            live_state = {
                "iteration": strategy.metrics['iterations'],
                "tool_calls": strategy.metrics['tool_calls'],
                "tokens": strategy.metrics['tokens']
            }
            
            # Execute ALL tool calls and collect results
            all_tool_actions = []
            all_tool_results = []
            total_reward = 0.0
            
            for tool_call in response['tool_calls']:
                if not strategy.can_use_tool():
                    break
                
                tool_name = tool_call['function']['name']
                try:
                    tool_args = json.loads(tool_call['function']['arguments'])
                except:
                    tool_args = {}
                
                result = execute_tool(tools, tool_name, tool_args)
                
                # Track test_code_snippet calls as compile attempts
                if tool_name == "test_code_snippet":
                    compile_attempts += 1
                    if isinstance(result, dict) and not result.get('compiles', False):
                        last_compile_errors = result.get('errors', [])
                    elif isinstance(result, dict) and 'error' in result:
                        last_compile_errors = [result['error']]
                
                # Prepare llm_view and full_result
                try:
                    if tool_name == 'test_code_snippet' and isinstance(result, dict):
                        # LLM view: filtered errors (2 start + 2 end)
                        llm_view = {
                            'compiles': result.get('compiles'),
                            'tests_pass': result.get('tests_pass'),
                            'test_summary': result.get('test_summary'),
                            'errors': result.get('errors', [])  # Already filtered by tool
                        }
                        # Full result: complete test output for trajectory storage
                        full_result = result.get('full_test_results', result)
                    else:
                        llm_view = result
                        full_result = result
                    trajectory_result = {'llm_view': llm_view, 'full_result': full_result}
                except Exception:
                    llm_view = result
                    full_result = result
                    trajectory_result = {'llm_view': llm_view, 'full_result': full_result}
                
                # Collect action and result
                all_tool_actions.append({
                    "tool": tool_name,
                    "args": tool_args,
                    "tool_call_id": tool_call.get('id', f'call_{len(trace)+len(all_tool_actions)}')
                })
                all_tool_results.append(trajectory_result)
                
                # Reward (RL)
                reward = 0.5 if (tool_name == "test_code_snippet" and "success" in str(result).lower()) else 0.1
                total_reward += reward
                
                # Show tool result preview
                result_preview = str(result)[:200]
                print(f"  ✓ Tool {len(all_tool_actions)}/{num_tool_calls}: {tool_name} - {result_preview}{'...' if len(str(result)) > 200 else ''}")
                
                # Log tool execution
                with open(log_file, 'a') as f:
                    f.write(f"TOOL CALL {len(all_tool_actions)}/{num_tool_calls}: {tool_name}\n")
                    f.write(f"Arguments: {json.dumps(tool_args, indent=2)}\n")
                    try:
                        f.write(f"LLM View: {json.dumps(llm_view, indent=2, default=str)}\n")
                    except Exception:
                        f.write(f"LLM View: {str(llm_view)}\n")
                    try:
                        f.write(f"Full Result: {json.dumps(full_result, indent=2, default=str)}\n")
                    except Exception:
                        f.write(f"Full Result: {str(full_result)}\n")
                    f.write(f"Result Preview: {str(llm_view)[:500]}{'...' if len(str(llm_view)) > 500 else ''}\n")
                    f.write(f"Result Size (LLM view): {len(str(llm_view))} chars\n")
                    f.write(f"Reward: {reward}\n\n")
                
                strategy.add_tool_call()
                print(f"🔧 Tool: {tool_name}({tool_args})")
                trace.append({
                    'iteration': strategy.metrics['iterations'],
                    'tool': tool_name,
                    'args': tool_args,
                    'result': trajectory_result
                })
                
                # RL trajectory
                rl_trajectory.append({"state": state, "action": {"tool": tool_name, "args": tool_args}, "reward": reward})
                
                # Add to conversation
                conversation.append({
                    "role": "assistant",
                    "content": response.get('content'),
                    "tool_calls": [tool_call]
                })
                # Prepare a privacy-preserving tool message content (LLM-facing)
                try:
                    tool_msg_content = json.dumps(llm_view, indent=2, default=str)
                except Exception:
                    tool_msg_content = str(llm_view)

                conversation.append({
                    "role": "tool",
                    "tool_call_id": tool_call.get('id', f'call_{len(trace)}'),
                    "content": tool_msg_content
                })
            
            # Create ONE trajectory step for ALL tool calls
            if trajectory and all_tool_actions:
                step_counter += 1
                
                # Calculate reasoning tokens
                if reasoning_content:
                    rc_tokens = len(reasoning_content) // 4
                elif reasoning_text:
                    rc_tokens = len(reasoning_text) // 4
                else:
                    rc_tokens = 0
                
                # Determine action type
                if len(all_tool_actions) == 1:
                    action_type = "tool_call"
                    action_data = all_tool_actions[0]
                else:
                    action_type = "multi_tool_call"
                    action_data = {"type": "multi_tool_call", "tools": all_tool_actions}
                
                # Determine result type
                if len(all_tool_results) == 1:
                    result_data = all_tool_results[0]
                else:
                    result_data = {"type": "multi_tool_result", "results": all_tool_results}
                
                trajectory.add_step(
                    step_id=step_counter,
                    state=live_state,
                    reasoning=reasoning_text,
                    action=action_data if len(all_tool_actions) == 1 else {"type": "multi_tool_call", "tools": all_tool_actions},
                    result=result_data,
                    api_status=api_status or "ok",
                    api_error=api_error or "na",
                    api_duration=api_duration,
                    tokens_used=tokens_used,  # Full token count for the API call
                    token_breakdown={
                        "completion_tokens": completion_tokens,
                        "prompt_tokens": prompt_tokens,
                        "total_tokens": total_tokens,
                        "reasoning_tokens": rc_tokens,
                        "tool_calls_count": len(all_tool_actions)
                    },
                    reasoning_content=reasoning_content if reasoning_content else None,
                    is_thinking_model=is_thinking_model
                )
            
            # Continue to next iteration after tool calls
            continue
        
        # Check for solution (only if no tool calls) - FIX 2: Compile first, then create trajectory step
        if response.get('content') and not response.get('tool_calls'):
            code = extract_code(response['content'])
            # Fallback: try reasoning_content if no code found (thinking models)
            if not code and response.get('reasoning_content'):
                code = extract_code(response['reasoning_content'])
            
            # Log extracted code
            with open(log_file, 'a') as f:
                f.write(f"\nEXTRACTED CODE ({len(code)} chars):\n")
                f.write("-"*80 + "\n")
                f.write(code if code else "[No code extracted]")
                f.write("\n" + "-"*80 + "\n\n")
            
            # Always add response to conversation first
            conversation.append({
                "role": "assistant",
                "content": response['content']
            })
            
            if code:  # Code extracted - compile FIRST, then create trajectory step
                # Validate compilation
                print(f"📦 Code extracted ({len(code)} chars), validating compilation...")
                compile_result = execute_tool(tools, "test_code_snippet", {"code": code})
                # Count this direct compile invocation as a tool call
                try:
                    strategy.add_tool_call()
                except Exception:
                    pass
                
                # Log compilation result
                with open(log_file, 'a') as f:
                    f.write(f"COMPILATION CHECK:\n")
                    f.write(json.dumps(compile_result, indent=2))
                    f.write("\n\n")
                
                compiles_flag = compile_result.get('compiles', False)
                tests_pass_flag = compile_result.get('tests_pass', None)

                # Prepare filtered tool result (LLM-facing view)
                try:
                    filtered_tool_result = {
                        k: v for k, v in compile_result.items()
                        if k not in ('full_test_results', 'test_env')
                    }
                except Exception:
                    filtered_tool_result = compile_result

                print(f'filtered_tool_result: {filtered_tool_result}')

                # For mistral models: skip tool message for implicit compilation (no actual tool call from model)
                # Deliver feedback as user message instead to avoid "Unexpected tool call id" error
                if 'mistral' in model.lower():
                    # Don't add tool message - feedback will be delivered via user message below if compilation fails
                    pass
                else:
                    # Add tool result to conversation for non-mistral models
                    try:
                        conversation.append({
                            "role": "tool",
                            "tool_call_id": f"test_code_snippet_{len(trace)+1}",
                            "content": json.dumps(filtered_tool_result, default=str)
                        })
                    except Exception:
                        try:
                            conversation.append({
                                "role": "tool",
                                "tool_call_id": f"test_code_snippet_{len(trace)+1}",
                                "content": str(filtered_tool_result)
                            })
                        except Exception:
                            pass

                # Build complete result with both LLM-facing and full views
                try:
                    llm_view = filtered_tool_result
                    full_result = compile_result.get('full_test_results') if isinstance(compile_result, dict) else compile_result
                except Exception:
                    llm_view = filtered_tool_result
                    full_result = compile_result

                # NOW create trajectory step with COMPLETE information
                if trajectory:
                    step_counter += 1
                    live_state = {
                        "iteration": strategy.metrics['iterations'],
                        "tool_calls": strategy.metrics['tool_calls'],
                        "tokens": strategy.metrics['tokens']
                    }
                    
                    # Calculate reasoning tokens
                    if reasoning_content:
                        rc_tokens = len(reasoning_content) // 4
                    elif reasoning_text:
                        rc_tokens = len(reasoning_text) // 4
                    else:
                        rc_tokens = 0
                    
                    # Create complete trajectory step
                    trajectory.add_step(
                        step_id=step_counter,
                        state=live_state,
                        reasoning=reasoning_text,
                        action={
                            "type": "code_submission",
                            "code": code,
                            "code_length": len(code)
                        },
                        result={
                            "compiles": compiles_flag,
                            "tests_pass": tests_pass_flag,
                            "test_summary": compile_result.get('test_summary'),
                            "errors": compile_result.get('errors', []),
                            "llm_view": llm_view,
                            "full_result": full_result
                        },
                        api_status=api_status or "ok",
                        api_error=api_error or "na",
                        api_duration=api_duration,
                        tokens_used=tokens_used,
                        token_breakdown={
                            "completion_tokens": completion_tokens,
                            "prompt_tokens": prompt_tokens,
                            "total_tokens": total_tokens,
                            "reasoning_tokens": rc_tokens
                        },
                        reasoning_content=reasoning_content if reasoning_content else None,
                        is_thinking_model=is_thinking_model
                    )
                    
                    # Store test results history
                    if not getattr(trajectory, 'test_results_history', None):
                        trajectory.test_results_history = []
                    trajectory.test_results_history.append({
                        'step_id': step_counter,
                        'llm_view': llm_view,
                        'full_result': full_result
                    })
                if compiles_flag and tests_pass_flag is True:
                    print(f"✅ Submission accepted (compiles successfully and tests passed)")
                    print(f"  Preview: {code[:100]}...")
                    strategy.metrics['stop_reason'] = 'submitted'

                    # Set final submission in trajectory
                    if trajectory:
                        trajectory.set_final_submission(
                            code=code,
                            compiled=True,
                            compile_error=None,
                            failure_category="success",
                            compile_attempts=compile_attempts,
                            last_compile_errors=[],
                            ast_score=ast_quality_score(code) if code else 0.0
                        )
                        trajectory.set_metrics(
                            total_steps=step_counter,
                            total_tool_calls=strategy.metrics['tool_calls'],
                            tokens_used=strategy.metrics['tokens'],
                            wall_clock_sec=time.time() - trajectory.start_time,
                            stop_reason='submitted'
                        )
                        # Persist generation-time full test artifacts into trajectory (if present)
                        try:
                            if isinstance(compile_result, dict):
                                full = compile_result.get('full_test_results')
                                if full:
                                    trajectory.test_results = full
                                    trajectory.passed_tests = (full.get('returncode') == 0)
                                    # Compute and persist code similarity (ground-truth vs agent code)
                                    try:
                                        agent_code = ''
                                        if getattr(trajectory, 'final_submission', None):
                                            fs = trajectory.final_submission
                                            if isinstance(fs, dict):
                                                agent_code = (fs.get('code') or '').strip()
                                        gt = extract_ground_truth_by_id(Path('benchmarks'), trajectory.repo, trajectory.task_id)
                                        if gt and agent_code:
                                            sim = None
                                            trajectory.code_similarity = {
                                                'cosine_similarity': sim,
                                                'ground_truth_length': len(gt),
                                                'agent_code_length': len(agent_code),
                                                'model': 'jinaai/jina-embeddings-v2-base-code'
                                            }
                                        else:
                                            trajectory.code_similarity = {
                                                'cosine_similarity': None,
                                                'ground_truth_length': len(gt) if gt else 0,
                                                'agent_code_length': len(agent_code) if agent_code else 0,
                                                'model': 'jinaai/jina-embeddings-v2-base-code'
                                            }
                                    except Exception:
                                        pass
                                    # If tests failed at runtime, update final_submission to reflect logic/test failure
                                    try:
                                        if getattr(trajectory, 'final_submission', None) and not trajectory.passed_tests:
                                            fs = trajectory.final_submission
                                            if isinstance(fs, dict):
                                                fs['failure_category'] = 'logic_error'
                                                # Populate last_compile_errors with runtime error lines for easier debugging
                                                fs['last_compile_errors'] = full.get('errors', []) or full.get('logs', [])
                                                # Also expose structured test failures at top-level
                                                try:
                                                    trajectory.test_failures = full.get('errors', []) or full.get('logs', [])
                                                except Exception:
                                                    pass
                                                fs['compile_error'] = None
                                                trajectory.final_submission = fs
                                    except Exception:
                                        pass
                        except Exception:
                            pass

                    return {
                        'code': code,
                        'metrics': strategy.get_metrics(),
                        'trace': trace,
                        'trajectory': trajectory
                    }

                # If compilation succeeded but tests failed or unknown, ask for a fix without leaking test output
                if compiles_flag and tests_pass_flag is not True:
                    print("⚠️  Compilation succeeded but tests did not pass — asking for fix")
                    # Add empty assistant message before user feedback (mistral models only)
                    if 'mistral' in model.lower():
                        conversation.append({"role": "assistant", "content": ""})
                    conversation.append({
                        "role": "user",
                        "content": (
                            "Compilation: SUCCESS\nTests: FAILED\n"
                            "Please analyze why tests failed (do NOT include test outputs). "
                            "Provide corrected code and, before the code, include a brief (<=2 sentences) "
                            "summary describing what you changed."
                        )
                    })
                    # Continue loop to receive corrected code
                    continue
                else:
                    # Compilation failed - provide feedback
                    compile_attempts += 1
                    errors = compile_result.get('errors', [])
                    last_compile_errors = errors  # Track most recent errors for trajectory
                    error_summary = '\n'.join(errors)  # Already truncated by tool to ~500 tokens
                    error_count = len(errors)
                    first_error_preview = errors[0][:100] if errors else 'No error details available'
                    
                    print(f"❌ Compilation failed: {error_count} error(s)")
                    print(f"  First error: {first_error_preview}")
                    # Add empty assistant message before user feedback (mistral models only)
                    if 'mistral' in model.lower():
                        conversation.append({
                            "role": "assistant",
                            "content": ""
                        })
                    conversation.append({
                        "role": "user",
                        "content": f"❌ COMPILATION FAILED\n\nCompiler errors ({len(errors)} total):\n{error_summary}\n\nAnalyze these errors carefully and fix your code. Pay attention to:\n- Missing includes or forward declarations\n- Type mismatches\n- Undefined members or methods\n- Template instantiation errors\n\nProvide corrected code."
                    })
            else:  # No code extracted
                # Create trajectory step for content-only response (no code)
                if trajectory:
                    step_counter += 1
                    live_state = {
                        "iteration": strategy.metrics['iterations'],
                        "tool_calls": strategy.metrics['tool_calls'],
                        "tokens": strategy.metrics['tokens']
                    }
                    
                    # Calculate reasoning tokens
                    if reasoning_content:
                        rc_tokens = len(reasoning_content) // 4
                    elif reasoning_text:
                        rc_tokens = len(reasoning_text) // 4
                    else:
                        rc_tokens = 0
                    
                    # Check if API returned error
                    step_result = {"code_extracted": False, "code_length": 0}
                    patched_status, patched_error = patch_api_status_error_from_result(step_result)
                    step_api_status = patched_status if patched_status else ("ok" if response.get('content') else "error")
                    step_api_error = patched_error if patched_error else (None if response.get('content') else "No content returned from API")
                    
                    if step_api_status == "error":
                        print(f"⚠️  Agent detected an API error: {step_api_error if step_api_error else 'Unknown error.'}.")
                    
                    trajectory.add_step(
                        step_id=step_counter,
                        state=live_state,
                        reasoning=reasoning_text,
                        action={"type": "content_only", "code_length": 0},
                        result=step_result,
                        api_status=step_api_status or "ok",
                        api_error=step_api_error or "na",
                        api_duration=api_duration,
                        tokens_used=tokens_used,
                        token_breakdown={
                            "completion_tokens": completion_tokens,
                            "prompt_tokens": prompt_tokens,
                            "total_tokens": total_tokens,
                            "reasoning_tokens": rc_tokens
                        },
                        reasoning_content=reasoning_content if reasoning_content else None,
                        is_thinking_model=is_thinking_model
                    )
                
                # Prompt for implementation
                conversation.append({
                    "role": "user",
                    "content": "Provide the complete C++ function implementation as a code block with no explanation."
                })
    
    # Budget exhausted
    metrics = strategy.get_metrics()
    print(f"⏰ Budget exhausted: {metrics['stop_reason']}")
    print(f"  Iterations: {metrics['iterations']}/{strategy_config['max_iterations']}")
    print(f"  Tool calls: {metrics['tool_calls']}/{strategy_config['max_tool_calls']}")
    print(f"  Tokens: {metrics['tokens']}/{strategy_config['max_tokens']}")
    print(f"  Time: {metrics['elapsed_time']:.1f}s/{strategy_config['max_time']}s")
    
    # Log final metrics
    with open(log_file, 'a') as f:
        f.write("\n" + "="*80 + "\n")
        f.write("FINAL METRICS:\n")
        f.write("-"*80 + "\n")
        final_metrics = strategy.get_metrics()
        f.write(json.dumps(final_metrics, indent=2))
        f.write("\n\n")
        f.write("BUDGET UTILIZATION:\n")
        f.write(f"  Tokens: {final_metrics['tokens']:,}/{strategy_config['max_tokens']:,} ({final_metrics['tokens']/strategy_config['max_tokens']*100:.1f}%)\n")
        f.write(f"  Iterations: {final_metrics['iterations']}/{strategy_config['max_iterations']} ({final_metrics['iterations']/strategy_config['max_iterations']*100:.1f}%)\n")
        f.write(f"  Tool Calls: {final_metrics['tool_calls']}/{strategy_config['max_tool_calls']} ({final_metrics['tool_calls']/strategy_config['max_tool_calls']*100:.1f}%)\n")
        f.write(f"  Time: {final_metrics['elapsed_time']:.1f}s/{strategy_config['max_time']}s ({final_metrics['elapsed_time']/strategy_config['max_time']*100:.1f}%)\n")
        f.write("\n" + "="*80 + "\n\n")
    
    if force_answer:
        print("🚨 Forcing final answer...")
        # Check last message role - if 'tool', add empty assistant message first (mistral models only)
        if 'mistral' in model.lower() and conversation and conversation[-1].get('role') == 'tool':
            conversation.append({"role": "assistant", "content": "Understood."})
        conversation.append({
            "role": "user",
            "content": "Budget exhausted. Provide your best solution NOW as C++ code."
        })
        print(f"  ⏳ Calling API for final answer...")
        response = call_api_with_tools(conversation, [], model, api_key, api_cooldown, error_log_file=error_log_file)
        code = extract_code(response.get('content', '') or '')
        # Fallback: try reasoning_content if no code found (thinking models)
        if not code and response.get('reasoning_content'):
            code = extract_code(response['reasoning_content'])
        # Validate extracted code is actually C++ (not JSON tool args or prose)
        if code and CppExtractor._is_json(code):
            print(f"⚠️  Forced answer produced JSON, not C++ — rejecting")
            code = ''
        if code and not CppExtractor._has_cpp_indicators(code):
            print(f"⚠️  Forced answer has no C++ indicators — rejecting")
            code = ''
        print(f"📝 Final code: {len(code)} chars")
        if code:
            print(f"  Preview: {code[:100]}...")
        
        # Compile the forced answer to check if it actually works
        forced_compiled = False
        forced_compile_error = None
        if code:
            print(f"  ⏳ Compiling forced answer...")
            forced_compile_result = execute_tool(tools, "test_code_snippet", {"code": code})
            if isinstance(forced_compile_result, dict):
                forced_compiled = forced_compile_result.get('compiles', False)
                if not forced_compiled:
                    forced_errors = forced_compile_result.get('errors', [])
                    forced_compile_error = '\n'.join(forced_errors) if forced_errors else None
                    last_compile_errors = forced_errors
            compile_attempts += 1
            if forced_compiled:
                print(f"  ✅ Forced answer compiles!")
            else:
                print(f"  ❌ Forced answer does not compile: {forced_compile_error}")
        
        # Log forced answer with extracted code
        with open(log_file, 'a') as f:
            f.write("FORCED FINAL ANSWER:\n")
            f.write(json.dumps(response, indent=2))
            f.write("\n\n")
            f.write(f"EXTRACTED CODE ({len(code)} chars):\n")
            f.write("-"*80 + "\n")
            f.write(code if code else "[No code extracted]")
            f.write("\n" + "-"*80 + "\n")
            if code:
                f.write(f"FORCED ANSWER COMPILATION: {'PASS' if forced_compiled else 'FAIL'}\n")
                if forced_compile_error:
                    f.write(f"Errors: {forced_compile_error}\n")
                f.write("\n")
    else:
        code = ''
        forced_compiled = False
        forced_compile_error = None
        print("⚠️  No forced answer - returning empty code")
    
    # Treat the forced final API response as a tool call and store it in the trajectory
    try:
        if force_answer and trajectory and response:
            # live state before recording this pseudo-tool call
            live_state = {
                "iteration": strategy.metrics['iterations'],
                "tool_calls": strategy.metrics['tool_calls'],
                "tokens": strategy.metrics['tokens']
            }

            # Build a consistent trajectory result structure
            forced_llm_view = response.get('content') if isinstance(response.get('content'), (str, dict)) else response.get('content')
            forced_full = response
            trajectory_result = {'llm_view': forced_llm_view, 'full_result': forced_full}

            # Tokens attribution: prefer usage info if available
            usage = response.get('usage', {}) if isinstance(response, dict) else {}
            comp_tokens = usage.get('completion_tokens', completion_tokens if 'completion_tokens' in locals() else 0)
            prompt_toks = usage.get('prompt_tokens', prompt_tokens if 'prompt_tokens' in locals() else 0)
            total_toks = usage.get('total_tokens', total_tokens if 'total_tokens' in locals() else 0)
            toks_used = comp_tokens if comp_tokens else total_toks

            # Reasoning content/tokens
            rc = reasoning_content if reasoning_content else None
            if rc:
                rc_tokens = (len(rc) // 4)
            elif reasoning_text:
                rc_tokens = (len(reasoning_text) // 4)
            else:
                rc_tokens = 0

            # Patch api_status/api_error from result if error info present
            patched_status, patched_error = patch_api_status_error_from_result(trajectory_result)
            step_api_status = patched_status if patched_status else api_status
            step_api_error = patched_error if patched_error else api_error
            # Record as a pseudo-tool call named 'forced_final_answer'
            step_counter += 1
            trajectory.add_step(
                step_id=step_counter,
                state=live_state,
                reasoning=reasoning_text,
                action={"tool": "forced_final_answer", "args": {}},
                result=trajectory_result,
                api_status=step_api_status or "ok",
                api_error=step_api_error or "na",
                api_duration=api_duration,
                tokens_used=toks_used,
                token_breakdown={
                    "completion_tokens": comp_tokens,
                    "prompt_tokens": prompt_toks,
                    "total_tokens": total_toks,
                    "reasoning_tokens": rc_tokens
                },
                reasoning_content=rc,
                is_thinking_model=is_thinking_model
            )

            # Update trace/rl and tool accounting to reflect this call
            try:
                trace.append({'iteration': strategy.metrics['iterations'], 'tool': 'forced_final_answer', 'args': {}, 'result': trajectory_result})
                rl_trajectory.append({"state": live_state, "action": {"tool": "forced_final_answer", "args": {}}, "reward": 0.0})
                strategy.add_tool_call()
            except Exception:
                pass
    except Exception:
        pass

    # Set final submission in trajectory (budget exhausted)
    if trajectory:
        final_code = code
        
        # Determine failure category based on actual compilation result
        if not final_code:
            failure_cat = "no_code_extracted"
        elif forced_compiled:
            failure_cat = "budget_exhausted_compiles"  # Code is valid, just ran out of budget
        else:
            failure_cat = "budget_exhausted"  # Code doesn't compile either
        trajectory.set_final_submission(
            code=final_code,
            compiled=forced_compiled,
            compile_error=forced_compile_error,
            failure_category=failure_cat,
            compile_attempts=compile_attempts,
            last_compile_errors=last_compile_errors,
            ast_score=ast_quality_score(final_code) if final_code else 0.0
        )
        trajectory.set_metrics(
            total_steps=step_counter,
            total_tool_calls=strategy.metrics['tool_calls'],
            tokens_used=strategy.metrics['tokens'],
            wall_clock_sec=time.time() - trajectory.start_time,
            stop_reason=metrics['stop_reason']
        )
        # If forced or final compile produced full test artifacts, persist them
        try:
            # Prefer forced_compile_result when available
            final_full = None
            if 'forced_compile_result' in locals() and isinstance(forced_compile_result, dict):
                final_full = forced_compile_result.get('full_test_results')
            elif 'compile_result' in locals() and isinstance(compile_result, dict):
                final_full = compile_result.get('full_test_results')

            if final_full:
                trajectory.test_results = final_full
                trajectory.passed_tests = (final_full.get('returncode') == 0)
                # Compute and persist code similarity (ground-truth vs agent code)
                try:
                    agent_code = ''
                    if getattr(trajectory, 'final_submission', None):
                        fs = trajectory.final_submission
                        if isinstance(fs, dict):
                            agent_code = (fs.get('code') or '').strip()
                    gt = extract_ground_truth_by_id(Path('benchmarks'), trajectory.repo, trajectory.task_id)
                    if gt and agent_code:
                        sim = None
                        trajectory.code_similarity = {
                            'cosine_similarity': sim,
                            'ground_truth_length': len(gt),
                            'agent_code_length': len(agent_code)
                        }
                    else:
                        trajectory.code_similarity = {
                            'cosine_similarity': None,
                            'ground_truth_length': len(gt) if gt else 0,
                            'agent_code_length': len(agent_code) if agent_code else 0
                        }
                except Exception:
                    pass
                # If tests failed at runtime, update final_submission to reflect logic/test failure
                try:
                    if getattr(trajectory, 'final_submission', None) and not trajectory.passed_tests:
                        fs = trajectory.final_submission
                        if isinstance(fs, dict):
                            fs['failure_category'] = 'logic_error'
                            fs['last_compile_errors'] = final_full.get('errors', []) or final_full.get('logs', [])
                            fs['compile_error'] = None
                            trajectory.final_submission = fs
                            try:
                                trajectory.test_failures = final_full.get('errors', []) or final_full.get('logs', [])
                            except Exception:
                                pass
                except Exception:
                    pass
        except Exception:
            pass
    
    return {
        'code': code,
        'metrics': strategy.get_metrics(),
        'trace': trace,
        'trajectory': trajectory
    }


def main():
    parser = argparse.ArgumentParser(description="Run agentic inference with inference provider API")
    # Required
    parser.add_argument('--model', default='qwen/qwen3.5-397b-a17b', help='Model name')
    parser.add_argument('--repo', action='append', help='Repository name(s) - can specify multiple times, or use "all"')
    parser.add_argument('--repos', help='Comma-separated list of repositories, or "all" for all repos')
    parser.add_argument('--setting', default='zero_shot', help='Setting')
    parser.add_argument('--functions', nargs='+', help='Specific function IDs to re-evaluate')
    parser.add_argument('--functions_file', help='JSON file with function errors (extracts exact settings)')
    # API
    parser.add_argument('--api_key', default=os.environ.get('INFERENCE_API_KEY', ''),
                        help='Inference provider API key (or set INFERENCE_API_KEY env var)')
    parser.add_argument('--api_keys', help='Comma-separated list of API keys for round-robin distribution across repos')
    parser.add_argument('--api_keys_file', help='File containing API keys (one per line) for round-robin distribution')
    parser.add_argument('--api_cooldown', type=float, default=2.0,
                        help='Seconds to wait between API calls (default 2.0s for 30 rpm)')
    # Strategy
    parser.add_argument('--agentic_strategy', default='hybrid',
                        choices=['iteration', 'token', 'time', 'hybrid'])
    # Budgets
    parser.add_argument('--max_iterations', type=int, default=12)
    parser.add_argument('--max_tool_calls', type=int, default=36)  # Reduced from 36 to encourage efficiency
    parser.add_argument('--max_time', type=int, default=600)  # 10 minutes (increased from 300s due to API timeouts)
    parser.add_argument('--max_tokens', type=int, default=100000)  # Increased for 200K context models
    parser.add_argument('--max_tokens_per_turn', type=int, default=7000)
    parser.add_argument('--tool_call_limit_per_iteration', type=int, default=4)
    parser.add_argument('--disable_compression', action='store_true', default=True,
                        help='Disable conversation compression (recommended for 200K context models)')
    # Tools
    parser.add_argument('--enable_web_search', action='store_true', default=False)
    parser.add_argument('--enable_cpp_docs', action='store_true', default=False)
    parser.add_argument('--enable_code_test', action='store_true', default=True)
    parser.add_argument('--enable_file_read', action='store_true', default=True)
    # Behavior
    parser.add_argument('--force_answer_on_budget_exhausted', action='store_true', default=True)
    parser.add_argument('--verbose', action='store_true')
    parser.add_argument('--limit', type=int, default=None, help='Limit number of functions to process')
    parser.add_argument('--beam_width', type=int, default=1, help='Number of solution attempts per task (1=single, 3=beam search)')
    parser.add_argument('--beam_temperature', type=float, default=0.0, help='Temperature for sampling (0.0=greedy/deterministic, 0.7=diverse for beam search)')
    parser.add_argument('--enable_thinking', action='store_true', default=False,
                        help='Force streaming mode to capture reasoning_content from thinking models')
    parser.add_argument('--skip_evaluation', action='store_true', default=False,
                        help='Do not auto-run scripts/evaluate_results.py after each repository')
    parser.add_argument('--resume', action='store_true', default=False,
                        help='Resume: skip tasks/beams with existing trajectory files')
    # ORM arguments
    parser.add_argument('--orm_checkpoint', type=str, default=None,
                        help='Path to ORM checkpoint for outcome scoring/reranking')
    parser.add_argument('--orm_device', type=str, default='cuda',
                        help='Device for ORM model (cuda or cpu)')
    parser.add_argument('--orm_rerank_beams', action='store_true', default=False,
                        help='Use ORM to rerank beam search final outcomes')
    args = parser.parse_args()
    
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
        print("❌ No repositories specified. Use --repo REPO, --repos repo1,repo2, or --repos all")
        sys.exit(1)
    
    repos = list(set(repos))  # Remove duplicates
    print(f"📦 Processing {len(repos)} repository(ies): {', '.join(repos)}")
    
    # Load functions file if provided
    functions_map = {}  # {task_id: {repo, setting}}
    if args.functions_file:
        with open(args.functions_file) as f:
            data = json.load(f)
        for model_entry in data.get('api_errors_main_modes_only', []):
            if model_entry['model'] == args.model:
                for err in model_entry['errors']:
                    functions_map[err['task_id']] = {
                        'repo': err['repo'],
                        'setting': err['setting']
                    }
        print(f"📋 Loaded {len(functions_map)} functions from {args.functions_file}")
    
    # Load ORM scorer if checkpoint provided
    orm_scorer = None
    if args.orm_checkpoint:
        print(f"\n🎯 Loading ORM from {args.orm_checkpoint}...")
        orm_scorer = load_orm_scorer(args.orm_checkpoint, device=args.orm_device)
        if orm_scorer:
            print(f"✓ ORM loaded successfully")
        else:
            print(f"⚠️  ORM loading failed, continuing without ORM")
    
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
        print("Set INFERENCE_PROVIDER and INFERENCE_API_KEY environment variables")
        print("\nUsage options:")
        print("  1. Single key: export INFERENCE_API_KEY='<api-key>...'")
        print("  2. Multiple keys: --api_keys 'key1,key2,key3'")
        print("  3. Keys file: --api_keys_file api_keys.txt")
        print("\nExample:")
        print("  python run_api_agent.py --model meta/llama-3.1-8b-instruct --repos all --api_keys 'key1,key2,key3'")
        sys.exit(1)
    
    print(f"\n🔑 Loaded {len(api_keys)} API key(s) for round-robin distribution")
    if len(api_keys) > 1:
        print(f"   Repositories will be processed in PARALLEL (one repo per API key)")
    else:
        print(f"   Repositories will be processed SEQUENTIALLY (single API key)")
    
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
    
    # Process repositories in parallel if multiple API keys, otherwise sequential
    if len(api_keys) > 1:
        print(f"\n🚀 Starting PARALLEL processing with {len(api_keys)} workers...\n")
        import multiprocessing as mp
        from multiprocessing import Process, Queue
        import queue
        
        # Worker function - processes multiple repos sequentially with one API key
        def api_key_worker(repos_list, api_key, api_key_idx, model, setting, beam_width, 
                          orm_checkpoint, orm_device, functions_map, worker_args, result_queue):
            """Worker that processes multiple repos sequentially with one API key"""
            try:
                # Reconstruct args
                import argparse
                args = argparse.Namespace(**worker_args)
                args.api_key = api_key
                
                # Load ORM scorer if needed (each process loads its own)
                orm_scorer = None
                if orm_checkpoint:
                    try:
                        orm_scorer = load_orm_scorer(orm_checkpoint, device=orm_device)
                    except Exception as e:
                        print(f"[Worker #{api_key_idx}] ⚠️  ORM loading failed: {e}")
                
                print(f"[Worker #{api_key_idx}] 🔑 Processing {len(repos_list)} repos sequentially")
                
                # Process each repo sequentially
                for repo_idx, repo in enumerate(repos_list, 1):
                    try:
                        # Create repo-specific log file
                        log_dir = Path(f"results/{model}/{repo}/{setting}")
                        log_dir.mkdir(parents=True, exist_ok=True)
                        repo_log = log_dir / f"repo_progress.log"
                        
                        with open(repo_log, 'w') as f:
                            import datetime
                            f.write(f"{'='*80}\n")
                            f.write(f"📦 REPOSITORY: {repo}\n")
                            f.write(f"🔑 API Key: #{api_key_idx}\n")
                            f.write(f"👷 Worker: #{api_key_idx} (Repo {repo_idx}/{len(repos_list)})\n")
                            f.write(f"⏰ Started: {datetime.datetime.now().isoformat()}\n")
                            f.write(f"{'='*80}\n\n")
                        
                        print(f"[Worker #{api_key_idx}] 📦 [{repo_idx}/{len(repos_list)}] Processing {repo}...")
                        
                        # Process repository
                        process_repository(repo, args, orm_scorer=orm_scorer, functions_map=functions_map)
                        
                        with open(repo_log, 'a') as f:
                            f.write(f"\n✅ Repository {repo} completed successfully\n")
                            f.write(f"⏰ Finished: {datetime.datetime.now().isoformat()}\n")
                        
                        # Auto-evaluate after repo completion
                        if not args.skip_evaluation:
                            print(f"[Worker #{api_key_idx}] 📈 Starting evaluation for {repo}...")
                            start_evaluation_in_screen(model, repo, setting, beam_width)
                        
                        result_queue.put({'repo': repo, 'status': 'success', 'error': None, 'worker': api_key_idx})
                        
                    except Exception as e:
                        import traceback
                        error_msg = traceback.format_exc()
                        
                        try:
                            log_dir = Path(f"results/{model}/{repo}/{setting}")
                            log_dir.mkdir(parents=True, exist_ok=True)
                            repo_log = log_dir / f"repo_progress.log"
                            
                            with open(repo_log, 'a') as f:
                                f.write(f"\n❌ Repository {repo} failed with error:\n")
                                f.write(error_msg)
                                f.write(f"\n⏰ Failed: {datetime.datetime.now().isoformat()}\n")
                        except:
                            pass
                        
                        print(f"[Worker #{api_key_idx}] ❌ Error processing {repo}: {e}")
                        result_queue.put({'repo': repo, 'status': 'failed', 'error': str(e), 'worker': api_key_idx})
                
                print(f"[Worker #{api_key_idx}] ✅ Completed all {len(repos_list)} repos")
                
            except Exception as e:
                import traceback
                print(f"[Worker #{api_key_idx}] ❌ Worker failed: {e}")
                print(traceback.format_exc())
        
        # Prepare worker arguments (must be picklable)
        worker_args = {
            'model': args.model,
            'setting': args.setting,
            'beam_width': args.beam_width,
            'beam_temperature': args.beam_temperature,
            'max_iterations': args.max_iterations,
            'max_tool_calls': args.max_tool_calls,
            'max_time': args.max_time,
            'max_tokens': args.max_tokens,
            'max_tokens_per_turn': args.max_tokens_per_turn,
            'tool_call_limit_per_iteration': args.tool_call_limit_per_iteration,
            'disable_compression': args.disable_compression,
            'enable_web_search': args.enable_web_search,
            'enable_cpp_docs': args.enable_cpp_docs,
            'enable_code_test': args.enable_code_test,
            'enable_file_read': args.enable_file_read,
            'force_answer_on_budget_exhausted': args.force_answer_on_budget_exhausted,
            'verbose': args.verbose,
            'limit': args.limit,
            'enable_thinking': args.enable_thinking,
            'skip_evaluation': args.skip_evaluation,
            'resume': args.resume,
            'orm_rerank_beams': args.orm_rerank_beams,
            'agentic_strategy': args.agentic_strategy,
            'api_cooldown': args.api_cooldown,
            'functions': args.functions,
            'functions_file': args.functions_file
        }
        
        # Group repos by API key
        key_to_repos = {}
        for repo in repos:
            api_key = repo_api_mapping[repo]['api_key']
            api_key_idx = repo_api_mapping[repo]['api_key_idx']
            if api_key not in key_to_repos:
                key_to_repos[api_key] = {'repos': [], 'idx': api_key_idx}
            key_to_repos[api_key]['repos'].append(repo)
        
        print(f"Worker Assignment:")
        print("-"*80)
        for api_key, data in key_to_repos.items():
            key_preview = api_key[:20] + '...' if len(api_key) > 20 else api_key
            print(f"  Worker #{data['idx']} ({key_preview}): {len(data['repos'])} repos")
            for repo in data['repos']:
                print(f"    - {repo}")
        print("-"*80 + "\n")
        
        # Create result queue
        result_queue = Queue()
        
        # Start one worker per API key
        processes = []
        for api_key, data in key_to_repos.items():
            repos_list = data['repos']
            api_key_idx = data['idx']
            
            p = Process(
                target=api_key_worker,
                args=(
                    repos_list,
                    api_key,
                    api_key_idx,
                    args.model,
                    args.setting,
                    args.beam_width,
                    args.orm_checkpoint,
                    args.orm_device,
                    functions_map,
                    worker_args,
                    result_queue
                )
            )
            p.start()
            processes.append((f"Worker #{api_key_idx}", p))
            print(f"🚀 Started Worker #{api_key_idx} (PID: {p.pid}) - {len(repos_list)} repos")
        
        # Monitor completion
        completed = 0
        failed = 0
        results = []
        
        while completed < len(repos):
            try:
                result = result_queue.get(timeout=10)
                results.append(result)
                completed += 1
                
                if result['status'] == 'success':
                    worker_id = result.get('worker', '?')
                    print(f"\n✅ [{completed}/{len(repos)}] {result['repo']} completed (Worker #{worker_id})")
                else:
                    failed += 1
                    worker_id = result.get('worker', '?')
                    print(f"\n❌ [{completed}/{len(repos)}] {result['repo']} failed (Worker #{worker_id}): {result['error']}")
            except queue.Empty:
                # Check if any process died
                for worker_name, p in processes:
                    if not p.is_alive() and p.exitcode is not None and p.exitcode != 0:
                        print(f"\n⚠️  {worker_name} died unexpectedly (exit code: {p.exitcode})")
        
        # Wait for all processes to finish
        for worker_name, p in processes:
            p.join(timeout=30)
            if p.is_alive():
                print(f"⚠️  Force terminating {worker_name}...")
                p.terminate()
                p.join()
        
        print(f"\n{'='*80}")
        print(f"✅ Parallel processing complete!")
        print(f"   Success: {completed - failed}/{len(repos)}")
        print(f"   Failed: {failed}/{len(repos)}")
        print(f"{'='*80}")
    else:
        # Sequential processing (single API key)
        print(f"\n📝 Starting SEQUENTIAL processing of {len(repos)} repositories...\n")
        for repo_idx, repo in enumerate(repos, 1):
            print(f"\n{'='*80}")
            print(f"📦 REPOSITORY [{repo_idx}/{len(repos)}]: {repo}")
            print(f"{'='*80}\n")
            
            # Get assigned API key for this repo
            assigned_api_key = repo_api_mapping[repo]['api_key']
            assigned_api_idx = repo_api_mapping[repo]['api_key_idx']
            
            print(f"🔑 Using API Key #{assigned_api_idx} for {repo}\n")
            
            # Override args.api_key with assigned key
            original_api_key = args.api_key
            args.api_key = assigned_api_key
            
            # Create repo-specific log file
            log_dir = Path(f"results/{args.model}/{repo}/{args.setting}")
            log_dir.mkdir(parents=True, exist_ok=True)
            repo_log = log_dir / f"repo_progress.log"
            
            with open(repo_log, 'w') as f:
                import datetime
                f.write(f"{'='*80}\n")
                f.write(f"📦 REPOSITORY: {repo}\n")
                f.write(f"🔑 API Key: #{assigned_api_idx}\n")
                f.write(f"⏰ Started: {datetime.datetime.now().isoformat()}\n")
                f.write(f"{'='*80}\n\n")
            
            try:
                process_repository(repo, args, orm_scorer=orm_scorer, functions_map=functions_map)
                
                with open(repo_log, 'a') as f:
                    f.write(f"\n✅ Repository {repo} completed successfully\n")
                    f.write(f"⏰ Finished: {datetime.datetime.now().isoformat()}\n")
            except Exception as e:
                import traceback
                with open(repo_log, 'a') as f:
                    f.write(f"\n❌ Repository {repo} failed with error:\n")
                    f.write(traceback.format_exc())
                    f.write(f"\n⏰ Failed: {datetime.datetime.now().isoformat()}\n")
                print(f"\n❌ Error processing {repo}: {e}")
            
            # Restore original
            args.api_key = original_api_key
            
            # Auto-evaluate after repo completion (optional)
            if not args.skip_evaluation:
                print(f"\n📈 Starting evaluation for {repo} in background...")
                start_evaluation_in_screen(args.model, repo, args.setting, args.beam_width)
            else:
                print(f"\nℹ️  Skipping auto-evaluation for {repo} (use scripts/evaluate_results.py manually)")
        
        print(f"\n✅ Completed all {len(repos)} repositories!")
    
    print(f"\nView running evaluations: screen -ls")
    print(f"Attach to evaluation: screen -r eval_{args.model.replace('/', '_')}")


def get_all_repos():
    """Get all available repositories from benchmarks directory"""
    benchmarks_dir = Path("benchmarks")
    if not benchmarks_dir.exists():
        return []
    repos = [d.name for d in benchmarks_dir.iterdir() 
             if d.is_dir() and (d / "metadata.jsonl").exists()]
    return sorted(repos)


def start_evaluation_in_screen(model, repo, setting, beam_width=1):
    """Start evaluate_results.py in a detached screen session"""
    import subprocess
    import datetime
    
    # Create screen session name
    timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    screen_name = f"eval_{model.replace('/', '_')}_{repo}_{timestamp}"
    
    # Create log directory
    log_dir = Path(f"results/{model}/{repo}/{setting}/eval_logs")
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"evaluation_{timestamp}.log"
    
    # Build evaluation command - test all beams if beam_width > 1
    if beam_width > 1:
        k_values = ','.join(str(i) for i in range(1, beam_width + 1))
        eval_cmd = f"python scripts/evaluate_results.py --model {model} --repo {repo} --setting {setting} --k {k_values}"
    else:
        eval_cmd = f"python scripts/evaluate_results.py --model {model} --repo {repo} --setting {setting} --k 1"
    
    # Start in screen with logging (auto-exit when done)
    screen_cmd = [
        "screen", "-dmS", screen_name,
        "bash", "-c",
        f"{eval_cmd} 2>&1 | tee {log_file}"
    ]
    
    try:
        subprocess.run(screen_cmd, check=True)
        print(f"  ✅ Started evaluation in screen: {screen_name}")
        print(f"  📝 Log file: {log_file}")
        print(f"  🔍 Attach: screen -r {screen_name}")
    except subprocess.CalledProcessError as e:
        print(f"  ⚠️  Failed to start screen session: {e}")
        print(f"  🔄 Running evaluation in foreground instead...")
        subprocess.run(eval_cmd.split(), check=False)


def rerank_beams_with_orm(beam_results, task_context, orm_scorer):
    """Rerank beam search FINAL outcomes using ORM scores.
    
    Args:
        beam_results: List of result dicts from solve_agentic_task
        task_context: Function signature + requirements
        orm_scorer: ORMScorer instance
        
    Returns:
        Reranked beam_results (best outcome first)
    """
    candidates = []
    for idx, result in enumerate(beam_results):
        # Extract COMPLETE reasoning chain from trajectory
        reasoning = ""
        if result.get('trajectory'):
            traj = result['trajectory']
            if hasattr(traj, 'steps'):
                # Concatenate ALL reasoning from all steps
                reasoning_parts = []
                for step in traj.steps:
                    if isinstance(step, dict) and step.get('reasoning'):
                        reasoning_parts.append(step['reasoning'])
                reasoning = "\n\n".join(reasoning_parts)
        
        # If no trajectory, try to extract from trace
        if not reasoning and result.get('trace'):
            reasoning_parts = []
            for step in result['trace']:
                if isinstance(step, dict) and step.get('reasoning'):
                    reasoning_parts.append(step['reasoning'])
            reasoning = "\n\n".join(reasoning_parts)
        
        candidates.append({
            'index': idx,
            'result': result,
            'reasoning': reasoning,
            'code': result.get('code', '')
        })
    
    # Score all FINAL outcomes
    reasonings = [c['reasoning'] for c in candidates]
    scores = orm_scorer.score_batch([task_context] * len(candidates), reasonings)
    
    # Add scores and sort
    for cand, score in zip(candidates, scores):
        cand['orm_score'] = score
        cand['result']['orm_score'] = score
    
    ranked = sorted(candidates, key=lambda x: x['orm_score'], reverse=True)
    
    # Log scores
    print(f"  ORM Scores (final outcomes):")
    for i, cand in enumerate(ranked):
        compiled = "✓" if cand['result'].get('metrics', {}).get('stop_reason') == 'submitted' else "✗"
        print(f"    Beam {cand['index']+1}: {cand['orm_score']:.3f} {compiled}")
    
    return [c['result'] for c in ranked]


def process_repository(repo, args, orm_scorer=None, functions_map=None):
    """Process a single repository"""
    
    # Load metadata
    metadata_path = f"benchmarks/{repo}/metadata.jsonl"
    if not Path(metadata_path).exists():
        print(f"❌ Metadata not found: {metadata_path}")
        sys.exit(1)
    
    with open(metadata_path) as f:
        tasks = [json.loads(line) for line in f]
    
    # Use empty dict if functions_map not provided
    if functions_map is None:
        functions_map = {}
    
    # Initialize tools
    tools = AgenticTools(
        repo_path=f"benchmarks/{repo}",
        enable_web=args.enable_web_search,
        enable_cpp_docs=args.enable_cpp_docs,
        enable_test=args.enable_code_test,
        enable_file_read=args.enable_file_read,
        mask_function_id=None  # Will be set per task
    )
    
    # Strategy config
    strategy_config = {
        'max_iterations': args.max_iterations,
        'max_tool_calls': args.max_tool_calls,
        'max_time': args.max_time,
        'max_tokens': args.max_tokens,
        'max_tokens_per_turn': args.max_tokens_per_turn,
        'tool_call_limit_per_iteration': args.tool_call_limit_per_iteration,
        'disable_compression': args.disable_compression
    }
    
    # Process tasks
    completions = []
    tasks_to_process = tasks[:args.limit] if args.limit else tasks
    
    # Filter by specific functions if provided
    if args.functions or args.functions_file:
        target_ids = set(args.functions or [])
        if args.functions_file and functions_map:
            target_ids.update(functions_map.keys())
        tasks_to_process = [t for t in tasks_to_process 
                           if t.get('function_id') in target_ids or t['namespace'] in target_ids]
        print(f"🎯 Filtered to {len(tasks_to_process)} specific functions")
        if len(tasks_to_process) == 0:
            print(f"⏭️  No matching functions in {repo}, skipping repository")
            return
    
    for i, task in enumerate(tasks_to_process):
        agentic_task = task.get('agentic_task', '')
        zero_shot = task.get('prompts', {}).get('zero_shot', '')
        if agentic_task and zero_shot:
            task_description = f"{agentic_task}\n\n=== DETAILED REQUIREMENTS ===\n{zero_shot}"
        else:
            task_description = agentic_task or zero_shot
        if not task_description:
            print(f"⏭️  Skipping {task['namespace']}: no task description")
            continue
        print(f"\n{'='*60}")
        print(f"📝 [{i+1}/{len(tasks_to_process)}] {task['namespace']}")
        print(f"{'='*60}")
        tools.mask_function_id = task.get('function_id', '')
        tools.current_task = task
        
        # Override setting if functions_file specifies exact setting
        task_id = task.get('function_id', task['namespace'])
        if args.functions_file and functions_map and task_id in functions_map:
            actual_setting = functions_map[task_id]['setting']
            print(f"📌 Using exact setting from JSON: {actual_setting}")
        else:
            actual_setting = args.setting
        
        # Build task context for ORM scoring (matches training format)
        sig = task.get('signature', '')
        req = task.get('requirement', {})
        func = req.get('Functionality', '') if isinstance(req, dict) else ''
        args_desc = req.get('Arguments', '') if isinstance(req, dict) else ''
        task_context = f"{sig}\n// {func}\n// Args: {args_desc}".strip()
        
        beam_results = []
        for beam_id in range(1, args.beam_width + 1):
            if args.beam_width > 1:
                print(f"\n🔹 Beam {beam_id}/{args.beam_width}")
            # Resume logic: skip only if trajectory exists AND all steps have api_status == 'ok'
            traj_path = TrajectoryBuilder.create_output_path(
                base_dir=Path("results"),
                repo=repo,
                task_id=task.get('function_id', task['namespace']),
                model=args.model,
                beam_id=beam_id,
                setting=actual_setting
            )
            skip_trajectory = False
            if args.resume and traj_path.exists():
                try:
                    with open(traj_path, 'r') as f:
                        trj_data = json.load(f)

                    steps = trj_data.get('trajectory', [])
                    api_error_found = any(step.get('api_status', 'ok') != 'ok' for step in steps)
                    
                    # Check stop_reason - if timeout, redo (STRICTER: also check test_res)
                    metrics = trj_data.get('metrics', {})
                    test_res = trj_data.get('passed_tests')
                    stop_reason = metrics.get('stop_reason')
                    is_timeout = (stop_reason == 'timeout' and not isinstance(test_res, bool))
                    
                    # Check for missing test results (NEW: from calculate_final_results.py)
                    failure_category = trj_data.get('final_submission', {}).get('failure_category')
                    missing_test_results = (not isinstance(test_res, bool) and failure_category != "no_code_extracted")
                    
                    # Check for llm_view.compiles bug (all patterns from BEFORE buggy code)
                    # BEFORE bug: compiles = len(filtered) == 0 (wrong - should use returncode)
                    # Bug Pattern 1: returncode=2, compiles=True (compilation failed, marked as success) - 729 cases
                    # Bug Pattern 2: returncode=0, compiles=False (compilation succeeded, marked as failed)
                    # Bug Pattern 3: returncode=1, compiles=False (compilation succeeded, tests failed, marked as failed)
                    compiles_bug_found = False
                    bug_details = []
                    
                    for step_idx, step in enumerate(steps):
                        result = step.get('result', {})
                        
                        # Check nested llm_view structure
                        if isinstance(result, dict) and 'llm_view' in result:
                            llm_view = result['llm_view']
                            full_result = result.get('full_result', {})
                            if isinstance(llm_view, dict) and isinstance(full_result, dict):
                                compiles = llm_view.get('compiles')
                                returncode = full_result.get('returncode')
                                
                                # Skip if returncode is None (not a compilation step)
                                if returncode is None:
                                    continue
                                
                                # Detect all bug patterns
                                if returncode == 0:
                                    # Success: compiles should be True
                                    if compiles is not True:
                                        compiles_bug_found = True
                                        bug_details.append(f"Step {step_idx}: returncode=0 but compiles={compiles} (Pattern 2)")
                                        break
                                
                                elif returncode == 1:
                                    # Tests failed: compiles should be True
                                    if compiles is not True:
                                        compiles_bug_found = True
                                        bug_details.append(f"Step {step_idx}: returncode=1 but compiles={compiles} (Pattern 3)")
                                        break
                                
                                elif returncode == 2:
                                    # Compilation failed: compiles should be False
                                    if compiles is True:
                                        compiles_bug_found = True
                                        bug_details.append(f"Step {step_idx}: returncode=2 but compiles={compiles} (Pattern 1)")
                                        break
                        
                        # Check flat structure (older format)
                        elif isinstance(result, dict):
                            compiles = result.get('compiles')
                            full_test = result.get('full_test_results', {})
                            if isinstance(full_test, dict):
                                returncode = full_test.get('returncode')
                                
                                if returncode is None:
                                    continue
                                
                                # Detect all bug patterns
                                if returncode == 0 and compiles is not True:
                                    compiles_bug_found = True
                                    bug_details.append(f"Step {step_idx}: returncode=0 but compiles={compiles} (Pattern 2, flat)")
                                    break
                                elif returncode == 1 and compiles is not True:
                                    compiles_bug_found = True
                                    bug_details.append(f"Step {step_idx}: returncode=1 but compiles={compiles} (Pattern 3, flat)")
                                    break
                                elif returncode == 2 and compiles is True:
                                    compiles_bug_found = True
                                    bug_details.append(f"Step {step_idx}: returncode=2 but compiles={compiles} (Pattern 1, flat)")
                                    break
                    
                    # Apply ALL filters (matching calculate_final_results.py logic)
                    if not api_error_found and not is_timeout and not missing_test_results and not compiles_bug_found:
                        skip_trajectory = True
                    else:
                        reasons = []
                        if api_error_found:
                            reasons.append("API error")
                        if is_timeout:
                            reasons.append("timeout")
                        if missing_test_results:
                            reasons.append("missing test results")
                        if compiles_bug_found:
                            reasons.append(f"llm_view.compiles bug ({bug_details[0] if bug_details else 'unknown pattern'})")
                        reason_str = ", ".join(reasons) if reasons else "unknown error"
                        print(f"⚠️  {reason_str} detected: redoing the trajectory for {task['namespace']} beam {beam_id}.")
                except Exception as e:
                    print(f"⚠️  Error reading trajectory for resume check: {e}")
                    skip_trajectory = False
            if skip_trajectory:
                print(f"⏭️ Skipped {task['namespace']} beam {beam_id}: trajectory already complete ({traj_path})")
                continue
            # Only create log file and run if not skipped
            actual_output_dir = Path(f"results/{args.model}/{repo}/{actual_setting}")
            actual_output_dir.mkdir(parents=True, exist_ok=True)
            if args.beam_width > 1:
                log_file = actual_output_dir / f"{task['namespace'].replace('::', '_')}_beam_{beam_id}_log.txt"
            else:
                log_file = actual_output_dir / f"{task['namespace'].replace('::', '_')}_log.txt"
            temperature = args.beam_temperature
            result = solve_agentic_task(
                task=task_description,
                namespace=task['namespace'],
                signature=task.get('signature', ''),
                tools=tools,
                strategy_name=args.agentic_strategy,
                strategy_config=strategy_config,
                model=args.model,
                api_key=args.api_key,
                force_answer=args.force_answer_on_budget_exhausted,
                log_file=log_file,
                api_cooldown=args.api_cooldown,
                repo_name=repo,
                task_id=task.get('function_id', task['namespace']),
                save_trajectory=True,
                temperature=temperature,
                enable_thinking=args.enable_thinking
            )
            if result.get('trajectory'):
                result['trajectory'].beam_id = beam_id
                result['trajectory'].save(traj_path)
                print(f"  💾 Saved trajectory: {traj_path.name}")
            beam_results.append(result)
        
        # ORM-based beam reranking (AFTER all beams complete)
        if args.orm_rerank_beams and orm_scorer and len(beam_results) > 1:
            print(f"\n🎯 Reranking {len(beam_results)} final outcomes with ORM...")
            beam_results = rerank_beams_with_orm(beam_results, task_context, orm_scorer)
            print(f"  ✓ Best outcome selected (ORM score: {beam_results[0].get('orm_score', 'N/A'):.3f})")
        
        # Only add completions if at least one beam was actually run (not skipped)
        if beam_results:
            completions.append({
                "namespace": task['namespace'],
                "completions": [r['code'] for r in beam_results],
                "agentic_metrics": beam_results[0]['metrics'],
                "agentic_trace": beam_results[0]['trace'],
                "beam_count": len(beam_results),
                "orm_scores": [r.get('orm_score') for r in beam_results] if orm_scorer else None
            })
    
    # Save completions (append/merge with existing completions.jsonl)
    # Use actual_setting from first task if available, otherwise use args.setting
    final_setting = args.setting
    if completions and functions_map:
        first_task_id = tasks_to_process[0].get('function_id', tasks_to_process[0]['namespace'])
        if first_task_id in functions_map:
            final_setting = functions_map[first_task_id]['setting']
    output_dir = Path(f"results/{args.model}/{repo}/{final_setting}")
    output_dir.mkdir(parents=True, exist_ok=True)
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
    # Merge: update/add new completions, keep old for skipped
    for comp in completions:
        existing_completions[comp['namespace']] = comp
    # Write merged completions back
    with open(output_file, 'w') as f:
        for comp in existing_completions.values():
            f.write(json.dumps(comp) + '\n')
    print(f"\n✅ Saved {len(existing_completions)} completions to {output_file}")


if __name__ == "__main__":
    main()