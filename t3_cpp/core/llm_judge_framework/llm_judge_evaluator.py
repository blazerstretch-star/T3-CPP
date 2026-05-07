#!/usr/bin/env python3
"""
LLM Judge Evaluator for Code Generation PRM
Async evaluation engine with caching, retry logic, and cost tracking.
"""

import asyncio
import hashlib
import json
import logging
import re
import sqlite3
import threading
import time
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass

import aiohttp
from json_repair import repair_json

from code_and_train_ds.t3_cpp.core.llm_judge_framework.llm_judge_personas_2axis import (
    EVALUATION_AXES_2AXIS as EVALUATION_AXES,
    route_evaluation_axes_2axis,
    get_axis_weights_2axis
)

logger = logging.getLogger("llm_judge")


# ═══════════════════════════════════════════════════════════════════════════
# CACHE MANAGEMENT
# ═══════════════════════════════════════════════════════════════════════════

class JudgeCache:
    """SQLite-based cache for LLM judge evaluations with thread-safe writes."""
    
    def __init__(self, cache_path: Path):
        self.cache_path = cache_path
        self._lock = threading.Lock()
        self.cache_path.parent.mkdir(parents=True, exist_ok=True)
        self.conn = sqlite3.connect(str(cache_path), check_same_thread=False)
        self._init_db()
    
    def _init_db(self):
        """Create cache table if not exists."""
        self.conn.execute("""
            CREATE TABLE IF NOT EXISTS judge_cache (
                cache_key TEXT PRIMARY KEY,
                axis TEXT NOT NULL,
                model TEXT NOT NULL,
                result TEXT NOT NULL,
                raw_response TEXT,
                timestamp INTEGER NOT NULL
            )
        """)
        self.conn.execute("""
            CREATE INDEX IF NOT EXISTS idx_axis_model 
            ON judge_cache(axis, model)
        """)
        self.conn.commit()
    
    def get(self, cache_key: str) -> Optional[Dict]:
        """Retrieve cached result (thread-safe)."""
        with self._lock:
            cursor = self.conn.execute(
                "SELECT result FROM judge_cache WHERE cache_key = ?",
                (cache_key,)
            )
            row = cursor.fetchone()
            if row:
                return json.loads(row[0])
            return None
    
    def set(self, cache_key: str, axis: str, model: str, result: Dict, raw_response: str = None):
        """Store result in cache (thread-safe)."""
        with self._lock:
            self.conn.execute(
                """
                INSERT OR REPLACE INTO judge_cache 
                (cache_key, axis, model, result, raw_response, timestamp)
                VALUES (?, ?, ?, ?, ?, ?)
                """,
                (cache_key, axis, model, json.dumps(result), raw_response, int(time.time()))
            )
            self.conn.commit()
    
    def stats(self) -> Dict[str, int]:
        """Get cache statistics."""
        with self._lock:
            cursor = self.conn.execute("SELECT COUNT(*) FROM judge_cache")
            total = cursor.fetchone()[0]
            
            cursor = self.conn.execute(
                "SELECT axis, COUNT(*) FROM judge_cache GROUP BY axis"
            )
            by_axis = dict(cursor.fetchall())
            
            return {"total": total, "by_axis": by_axis}
    
    def close(self):
        """Close database connection."""
        self.conn.close()


def compute_cache_key(
    step: Dict,
    axis: str,
    model: str,
    context_steps: Optional[List[Dict]] = None
) -> str:
    """
    Generate deterministic cache key for a step evaluation.
    
    Uses last 3 context steps to match prompt construction.
    """
    task_id = step.get("task_id", "")
    step_position = step.get("step_position", 0)
    action_type = step.get("action_type", "")
    tool_name = step.get("tool_name", "")
    
    # Include reasoning snippet directly (no intermediate hash)
    reasoning_snippet = step.get("reasoning_normalized", "")[:200]
    
    # Use last 3 context steps only (matches prompt construction)
    context_hash = ""
    if context_steps:
        recent_steps = context_steps[-3:]
        context_parts = []
        for s in recent_steps:
            # Fallback to task_id + step_position if content_hash missing
            ch = s.get("content_hash", "")
            if not ch:
                ch = f"{s.get('task_id', '')}_{s.get('step_position', '')}"
            context_parts.append(ch)
        context_hash = "|".join(context_parts)
    
    obs = step.get("observation", {})
    obs_str = json.dumps(obs, sort_keys=True, default=str)
    
    # Single hash of all components
    key_string = f"{axis}|{model}|{task_id}|{step_position}|{reasoning_snippet}|{action_type}|{tool_name}|{context_hash}|{obs_str}"
    return hashlib.md5(key_string.encode()).hexdigest()


# ═══════════════════════════════════════════════════════════════════════════
# LLM API CLIENTS
# ═══════════════════════════════════════════════════════════════════════════

@dataclass
class APIConfig:
    """Configuration for LLM API."""
    provider: str
    model: str
    api_key: Optional[str] = None
    base_url: Optional[str] = None
    temperature: float = 0.0
    max_tokens: int = 16384
    timeout: int = 240  # Base timeout 240s (4 minutes), increases to 360s, 480s on retries
    rpm_limit: int = 35  # Rate limit: requests per minute per API key
    # Fallback model configuration
    fallback_model: Optional[str] = None  # e.g., "meta/llama-3.1-405b-instruct"
    fallback_provider: Optional[str] = None  # e.g., "nvidia" (defaults to primary provider)
    fallback_api_key: Optional[str] = None  # API key for fallback provider (defaults to primary key)
    fallback_temperature: float = 0.2
    fallback_max_tokens: int = 1024


# Pricing per 1M tokens (input / output separate)
PRICING = {
    "gpt-4o-mini":       {"input": 0.15,  "output": 0.60},
    "gpt-4o":            {"input": 2.50,  "output": 10.00},
    "claude-3-5-haiku":  {"input": 0.80,  "output": 4.00},
    "claude-3-5-sonnet": {"input": 3.00,  "output": 15.00},
    # NVIDIA NIM pricing (approximate)
    "meta/llama-3.1-8b-instruct": {"input": 0.20, "output": 0.20},
    "meta/llama-3.1-70b-instruct": {"input": 0.60, "output": 0.60},
    "meta/llama-3.1-405b-instruct": {"input": 1.20, "output": 1.20},
    "mistralai/mixtral-8x7b-instruct-v0.1": {"input": 0.50, "output": 0.50},
    "nvidia/llama-3.1-nemotron-ultra-253b-v1": {"input": 1.00, "output": 1.00},
}


class LLMClient:
    """Async LLM API client with retry logic, rate limiting, and fallback model support."""
    
    # Class-level lock for Windows overflow logging (no fcntl)
    _overflow_write_lock = threading.Lock()
    
    def __init__(self, config: APIConfig, api_keys: Optional[List[str]] = None):
        self.config = config
        self.api_keys = api_keys or [config.api_key]
        self.current_key_index = 0
        self.key_lock = asyncio.Lock()
        self.session: Optional[aiohttp.ClientSession] = None
        self.total_input_tokens = 0
        self.total_output_tokens = 0
        self.total_cost = 0.0
        self.token_lock = asyncio.Lock()  # Protect token counters
        
        # Fallback model tracking
        self.fallback_used_count = 0
        self.primary_success_count = 0
        self.primary_failure_count = 0
        
        # Rate limiting: track last request time per API key with per-key locks
        self.last_request_time = {i: 0.0 for i in range(len(self.api_keys))}
        self.rate_limit_locks = {i: asyncio.Lock() for i in range(len(self.api_keys))}
        self.min_interval = 60.0 / config.rpm_limit  # Seconds between requests
    
    async def _update_token_counts(self, input_tokens: int, output_tokens: int):
        """Update token counts and cost tracking (thread-safe)."""
        pricing = PRICING.get(self.config.model, {"input": 0, "output": 0})
        cost = (input_tokens * pricing["input"] + output_tokens * pricing["output"]) / 1_000_000
        
        async with self.token_lock:
            self.total_input_tokens += input_tokens
            self.total_output_tokens += output_tokens
            self.total_cost += cost
    
    async def _get_next_api_key(self) -> Tuple[str, int]:
        """Round-robin API key selection. Returns (key, index)."""
        async with self.key_lock:
            key_index = self.current_key_index
            key = self.api_keys[key_index]
            self.current_key_index = (self.current_key_index + 1) % len(self.api_keys)
            return key, key_index
    
    async def _wait_for_rate_limit(self, key_index: int):
        """Wait if necessary to respect rate limit for this API key.
        
        Must be called inside rate_limit_locks[key_index] context.
        """
        current_time = time.time()
        time_since_last = current_time - self.last_request_time[key_index]
        
        if time_since_last < self.min_interval:
            wait_time = self.min_interval - time_since_last
            logger.debug(f"Rate limiting key {key_index}: waiting {wait_time:.2f}s")
            await asyncio.sleep(wait_time)
        
        self.last_request_time[key_index] = time.time()
    
    async def __aenter__(self):
        self.session = aiohttp.ClientSession()
        return self
    
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.session:
            await self.session.close()
    
    async def call(
        self,
        system_prompt: str,
        user_prompt: str,
        max_retries: int = 3
    ) -> Tuple[str, int, int]:
        """
        Call LLM API with retry logic, dynamic timeout increase, and fallback model support.
        
        If primary model fails after all retries, automatically switches to fallback model.
        
        Returns:
            (response_text, input_tokens, output_tokens)
        """
        base_timeout = self.config.timeout
        
        # Try primary model with retries
        for attempt in range(max_retries):
            try:
                # Increase timeout on each retry: base, base*1.5, base*2
                current_timeout = int(base_timeout * (1 + 0.5 * attempt))
                
                if self.config.provider == "openai":
                    result = await self._call_openai(system_prompt, user_prompt, current_timeout)
                elif self.config.provider == "anthropic":
                    result = await self._call_anthropic(system_prompt, user_prompt, current_timeout)
                elif self.config.provider == "ollama":
                    result = await self._call_ollama(system_prompt, user_prompt, current_timeout)
                elif self.config.provider == "nvidia":
                    result = await self._call_nvidia(system_prompt, user_prompt, current_timeout)
                else:
                    raise ValueError(f"Unknown provider: {self.config.provider}")
                
                # Success with primary model
                self.primary_success_count += 1
                return result
            
            except (aiohttp.ClientError, asyncio.TimeoutError, ValueError) as e:
                if attempt == max_retries - 1:
                    # All retries exhausted - try fallback if configured
                    self.primary_failure_count += 1
                    
                    if self.config.fallback_model:
                        logger.warning(
                            f"Primary model {self.config.model} failed after {max_retries} attempts. "
                            f"Switching to fallback model {self.config.fallback_model}"
                        )
                        try:
                            result = await self._call_fallback_model(system_prompt, user_prompt)
                            self.fallback_used_count += 1
                            logger.info(f"✓ Fallback model succeeded (total fallbacks: {self.fallback_used_count})")
                            return result
                        except Exception as fallback_error:
                            logger.error(
                                f"Fallback model {self.config.fallback_model} also failed: "
                                f"{type(fallback_error).__name__}: {str(fallback_error)}"
                            )
                            raise  # Re-raise fallback error
                    else:
                        logger.error(f"Primary model failed and no fallback configured: {type(e).__name__}: {str(e)}")
                        raise
                
                logger.warning(
                    f"API call attempt {attempt + 1} failed: {type(e).__name__}: {str(e)}, "
                    f"retrying with {int(base_timeout * (1 + 0.5 * (attempt + 1)))}s timeout..."
                )
                await asyncio.sleep(2 ** attempt)
        
        raise RuntimeError("Unreachable")
    
    async def _call_openai(self, system_prompt: str, user_prompt: str, timeout: int) -> Tuple[str, int, int]:
        """Call OpenAI API."""
        url = self.config.base_url or "https://api.openai.com/v1/chat/completions"
        
        # Get next API key and apply rate limiting
        api_key, key_index = await self._get_next_api_key()
        async with self.rate_limit_locks[key_index]:
            await self._wait_for_rate_limit(key_index)
        
        headers = {
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json"
        }
        payload = {
            "model": self.config.model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            "temperature": self.config.temperature,
            "max_tokens": self.config.max_tokens
        }
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=timeout)
        ) as resp:
            resp.raise_for_status()
            data = await resp.json()
            
            content = data["choices"][0]["message"]["content"]
            input_tokens = data["usage"]["prompt_tokens"]
            output_tokens = data["usage"]["completion_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens
    
    async def _call_anthropic(self, system_prompt: str, user_prompt: str, timeout: int) -> Tuple[str, int, int]:
        """Call Anthropic API."""
        url = self.config.base_url or "https://api.anthropic.com/v1/messages"
        
        # Get next API key and apply rate limiting
        api_key, key_index = await self._get_next_api_key()
        async with self.rate_limit_locks[key_index]:
            await self._wait_for_rate_limit(key_index)
        
        headers = {
            "x-api-key": api_key,
            "anthropic-version": "2023-06-01",
            "Content-Type": "application/json"
        }
        payload = {
            "model": self.config.model,
            "system": system_prompt,
            "messages": [{"role": "user", "content": user_prompt}],
            "temperature": self.config.temperature,
            "max_tokens": self.config.max_tokens
        }
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=timeout)
        ) as resp:
            resp.raise_for_status()
            data = await resp.json()
            
            content = data["content"][0]["text"]
            input_tokens = data["usage"]["input_tokens"]
            output_tokens = data["usage"]["output_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens
    
    async def _call_ollama(self, system_prompt: str, user_prompt: str, timeout: int) -> Tuple[str, int, int]:
        """Call Ollama API."""
        url = self.config.base_url or "http://localhost:11434/api/chat"
        payload = {
            "model": self.config.model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            "stream": False,
            "options": {
                "temperature": self.config.temperature,
                "num_predict": self.config.max_tokens
            }
        }
        
        async with self.session.post(
            url, json=payload, timeout=aiohttp.ClientTimeout(total=timeout)
        ) as resp:
            resp.raise_for_status()
            data = await resp.json()
            
            content = data["message"]["content"]
            input_tokens = data.get("prompt_eval_count", 0)
            output_tokens = data.get("eval_count", 0)
            
            if input_tokens == 0 or output_tokens == 0:
                logger.warning(f"Ollama returned 0 tokens (input={input_tokens}, output={output_tokens})")
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens
    
    async def _call_nvidia(self, system_prompt: str, user_prompt: str, timeout: int) -> Tuple[str, int, int]:
        """Call NVIDIA NIM API with thinking mode support and context overflow handling."""
        url = self.config.base_url or "https://integrate.api.nvidia.com/v1/chat/completions"
        
        # Get next API key (round-robin)
        api_key, key_index = await self._get_next_api_key()
        
        # Rate limit: lock only for timing check and timestamp update
        async with self.rate_limit_locks[key_index]:
            await self._wait_for_rate_limit(key_index)
        # Lock released before request
        
        headers = {
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json"
        }
        
        # Check if this is a thinking model (nemotron-ultra)
        is_thinking_model = "nemotron-ultra" in self.config.model
        
        # For thinking models, prepend "detailed thinking on" to system prompt
        if is_thinking_model:
            system_prompt = "detailed thinking on\n\n" + system_prompt
        
        payload = {
            "model": self.config.model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            "temperature": self.config.temperature if self.config.temperature > 0 else 0.6,
            "top_p": 0.95,
            "max_tokens": self.config.max_tokens,
            "frequency_penalty": 0,
            "presence_penalty": 0,
            "stream": False,
            "response_format": {"type": "json_object"}  # Force JSON output
        }
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=timeout)
        ) as resp:
            if resp.status == 400:
                error_text = await resp.text()
                if "context length" in error_text.lower() or "too large" in error_text.lower():
                    logger.error(f"Context overflow (130K+ tokens): {error_text}")
                    await self._log_context_overflow(user_prompt)
                    raise ValueError(f"Context length exceeded: {error_text}")
                logger.error(f"NVIDIA API error {resp.status}: {error_text}")
                resp.raise_for_status()
            elif resp.status != 200:
                error_text = await resp.text()
                logger.error(f"NVIDIA API error {resp.status}: {error_text}")
                resp.raise_for_status()
            
            data = await resp.json()
            
            if "choices" not in data or len(data["choices"]) == 0:
                logger.error(f"Invalid response structure: {data}")
                raise ValueError(f"Invalid API response: {data}")
            
            message = data["choices"][0].get("message", {})
            content = message.get("content") or ""
            reasoning_content = message.get("reasoning_content") or ""
            response_text = reasoning_content if reasoning_content else content
            
            if not response_text:
                logger.error(f"Both content and reasoning_content are empty. Full response: {data}")
                raise ValueError(f"API returned empty content. Response: {data}")
            
            input_tokens = data["usage"]["prompt_tokens"]
            output_tokens = data["usage"]["completion_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return response_text, input_tokens, output_tokens
    
    async def _log_context_overflow(self, user_prompt: str):
        """Log trajectory that exceeded context length (async, non-blocking)."""
        try:
            match = re.search(r'\*\*Task\*\*: (\S+)', user_prompt)
            task_id = match.group(1) if match else "unknown"
            
            overflow_file = Path("context_overflow_trajectories.jsonl")
            
            log_entry = json.dumps({
                "task_id": task_id,
                "timestamp": int(time.time()),
                "prompt_length": len(user_prompt),
                "estimated_tokens": len(user_prompt) // 4
            }) + "\n"
            
            # Use executor for blocking file I/O
            loop = asyncio.get_event_loop()
            await loop.run_in_executor(None, self._write_overflow_log, overflow_file, log_entry)
            
            logger.info(f"Logged context overflow for task {task_id} to {overflow_file}")
        except Exception as e:
            logger.warning(f"Failed to log context overflow: {e}")
    
    def _write_overflow_log(self, overflow_file: Path, log_entry: str):
        """Blocking file write (runs in executor)."""
        try:
            import fcntl
            with open(overflow_file, "a") as f:
                fcntl.flock(f.fileno(), fcntl.LOCK_EX)
                f.write(log_entry)
                fcntl.flock(f.fileno(), fcntl.LOCK_UN)
        except ImportError:
            # Windows fallback (no fcntl) - use class-level lock
            with LLMClient._overflow_write_lock:
                with open(overflow_file, "a") as f:
                    f.write(log_entry)
    
    async def _call_fallback_model(
        self, 
        system_prompt: str, 
        user_prompt: str
    ) -> Tuple[str, int, int]:
        """Call fallback model with optimized parameters."""
        fallback_provider = self.config.fallback_provider or self.config.provider
        
        # Warn if fallback provider differs from primary
        if fallback_provider != self.config.provider:
            logger.warning(
                f"Fallback provider '{fallback_provider}' differs from primary '{self.config.provider}'"
            )
        
        # Route to appropriate provider
        if fallback_provider == "nvidia":
            return await self._call_fallback_nvidia(system_prompt, user_prompt)
        elif fallback_provider == "openai":
            return await self._call_fallback_openai(system_prompt, user_prompt)
        elif fallback_provider == "anthropic":
            return await self._call_fallback_anthropic(system_prompt, user_prompt)
        else:
            raise ValueError(f"Unsupported fallback provider: {fallback_provider}")
    
    async def _call_fallback_nvidia(
        self,
        system_prompt: str,
        user_prompt: str
    ) -> Tuple[str, int, int]:
        """Call NVIDIA fallback model."""
        url = self.config.base_url or "https://integrate.api.nvidia.com/v1/chat/completions"
        
        # Use fallback key if configured, otherwise use round-robin from primary keys
        if self.config.fallback_api_key:
            api_key = self.config.fallback_api_key
            # Fallback key doesn't use rate limiting (separate key pool)
        else:
            api_key, key_index = await self._get_next_api_key()
            # Apply rate limiting for primary keys
            async with self.rate_limit_locks[key_index]:
                await self._wait_for_rate_limit(key_index)
        
        headers = {
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json"
        }
        
        payload = {
            "model": self.config.fallback_model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            "temperature": self.config.fallback_temperature,
            "top_p": 0.7,
            "max_tokens": self.config.fallback_max_tokens,
            "stream": False
        }
        
        timeout = 120
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=timeout)
        ) as resp:
            if resp.status != 200:
                error_text = await resp.text()
                logger.error(f"Fallback model API error {resp.status}: {error_text}")
                resp.raise_for_status()
            
            data = await resp.json()
            
            if "choices" not in data or len(data["choices"]) == 0:
                logger.error(f"Invalid fallback response structure: {data}")
                raise ValueError(f"Invalid fallback API response: {data}")
            
            message = data["choices"][0].get("message", {})
            content = message.get("content", "")
            reasoning_content = message.get("reasoning_content", "")
            if not content and reasoning_content:
                content = reasoning_content
            
            if not content:
                logger.error(f"Fallback model returned empty content. Full response: {data}")
                raise ValueError(f"Fallback API returned empty content: {data}")
            
            input_tokens = data["usage"]["prompt_tokens"]
            output_tokens = data["usage"]["completion_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens
    
    async def _call_fallback_openai(
        self,
        system_prompt: str,
        user_prompt: str
    ) -> Tuple[str, int, int]:
        """Call OpenAI fallback model."""
        url = self.config.base_url or "https://api.openai.com/v1/chat/completions"
        api_key = self.config.fallback_api_key or self.config.api_key
        
        headers = {
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json"
        }
        payload = {
            "model": self.config.fallback_model,
            "messages": [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt}
            ],
            "temperature": self.config.fallback_temperature,
            "max_tokens": self.config.fallback_max_tokens
        }
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=120)
        ) as resp:
            resp.raise_for_status()
            data = await resp.json()
            
            content = data["choices"][0]["message"]["content"]
            input_tokens = data["usage"]["prompt_tokens"]
            output_tokens = data["usage"]["completion_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens
    
    async def _call_fallback_anthropic(
        self,
        system_prompt: str,
        user_prompt: str
    ) -> Tuple[str, int, int]:
        """Call Anthropic fallback model."""
        url = self.config.base_url or "https://api.anthropic.com/v1/messages"
        api_key = self.config.fallback_api_key or self.config.api_key
        
        headers = {
            "x-api-key": api_key,
            "anthropic-version": "2023-06-01",
            "Content-Type": "application/json"
        }
        payload = {
            "model": self.config.fallback_model,
            "system": system_prompt,
            "messages": [{"role": "user", "content": user_prompt}],
            "temperature": self.config.fallback_temperature,
            "max_tokens": self.config.fallback_max_tokens
        }
        
        async with self.session.post(
            url, headers=headers, json=payload, timeout=aiohttp.ClientTimeout(total=120)
        ) as resp:
            resp.raise_for_status()
            data = await resp.json()
            
            content = data["content"][0]["text"]
            input_tokens = data["usage"]["input_tokens"]
            output_tokens = data["usage"]["output_tokens"]
            
            await self._update_token_counts(input_tokens, output_tokens)
            
            return content, input_tokens, output_tokens


# ═══════════════════════════════════════════════════════════════════════════
# PROMPT CONSTRUCTION
# ═══════════════════════════════════════════════════════════════════════════

def smart_truncate(text: str, max_chars: int, by: str = "sentence") -> str:
    """
    Intelligently truncate text at sentence or line boundaries.
    
    Args:
        text: Text to truncate
        max_chars: Maximum characters
        by: "sentence" or "line"
    
    Returns:
        Truncated text with marker
    """
    if len(text) <= max_chars:
        return text
    
    truncated = text[:max_chars]
    
    if by == "sentence":
        # Find last sentence boundary
        last_end = max(truncated.rfind('. '), truncated.rfind('.\n'))
        # Only use boundary if it's not too far back (at least halfway)
        if last_end > max_chars // 2:
            return truncated[:last_end + 1] + " [truncated]"
        return truncated + " [truncated]"
    
    elif by == "line":
        # Find last line boundary
        last_newline = truncated.rfind('\n')
        if last_newline > max_chars // 2:
            return truncated[:last_newline] + "\n// [truncated]"
        return truncated + "\n// [truncated]"
    
    else:
        raise ValueError(f"Unknown truncation mode: {by}")


def summarize_observation(obs: Dict) -> str:
    """Pre-process observation into readable summary."""
    status = obs.get("status", "unknown")
    exit_code = obs.get("exit_code")
    stdout = str(obs.get("stdout", ""))
    stderr = str(obs.get("stderr", ""))
    tests_passed = obs.get("tests_passed")
    
    parts = [f"Status: {status}"]
    if exit_code is not None:
        parts.append(f"Exit: {exit_code}")
    if tests_passed is not None:
        parts.append(f"Tests: {tests_passed}")
    if stdout:
        parts.append(f"Stdout: {stdout[-300:]}")
    if stderr:
        parts.append(f"Stderr: {stderr[-200:]}")
    return " | ".join(parts)


def build_user_prompt(
    step: Dict,
    axis: str,
    context_steps: Optional[List[Dict]] = None
) -> str:
    """Build user prompt with smart context truncation."""
    task_id = step.get("task_id", "unknown")
    repo = step.get("repo", "unknown")
    step_position = step.get("step_position", 0)
    remaining_steps = max(0, step.get("remaining_steps", 0))
    total_steps = max(step_position + remaining_steps + 1, step.get("total_steps", 1))
    is_forced_final = step.get("is_forced_final", False)
    
    action = step.get("action") or {}
    action_type = action.get("type", "unknown")
    tool_name = action.get("tool_name", "")
    
    reasoning = step.get("reasoning_normalized", "") or step.get("reasoning", "")
    code = step.get("code", "") or action.get("code", "")
    content = action.get("content", "")
    observation = step.get("observation") or {}
    
    # Budget context
    budget_pct = int((step_position / total_steps) * 100) if total_steps > 0 else 0
    is_final = remaining_steps == 0
    
    # Build context summary - last 3 steps only, with truncation
    context_summary = "N/A"
    if context_steps:
        recent_steps = context_steps[-3:]  # Last 3 only
        context_lines = []
        for i, ctx_step in enumerate(recent_steps):
            ctx_action = ctx_step.get("action") or {}
            ctx_type = ctx_action.get("type", "")
            ctx_tool = ctx_action.get("tool_name", "")
            ctx_reasoning = ctx_step.get("reasoning_normalized", "")
            ctx_code = ctx_step.get("code", "") or ctx_action.get("code", "")
            ctx_obs = ctx_step.get("observation") or {}
            
            # Truncate old steps more aggressively
            is_recent = i >= len(recent_steps) - 2  # Last 2 steps
            if is_recent:
                reasoning_display = ctx_reasoning
            else:
                reasoning_display = ctx_reasoning[:100] + "..." if len(ctx_reasoning) > 100 else ctx_reasoning
            
            # Code summary: signatures only for old steps
            if ctx_code and not is_recent:
                lines = ctx_code.split('\n')
                code_display = '\n'.join(lines[:3] + ['...'] + lines[-3:]) if len(lines) > 6 else ctx_code
            else:
                code_display = ctx_code[:500] + "..." if len(ctx_code) > 500 else ctx_code
            
            obs_display = summarize_observation(ctx_obs)
            
            ctx_summary = (
                f"  Step {ctx_step.get('step_position', '?')}: {ctx_type} ({ctx_tool or 'none'})\n"
                f"  Reasoning: {reasoning_display}\n"
            )
            if code_display:
                ctx_summary += f"  Code: {code_display}\n"
            ctx_summary += f"  Result: {obs_display}"
            
            context_lines.append(ctx_summary)
        context_summary = "\n\n".join(context_lines)
    
    # Current step: full content
    reasoning_display = reasoning if reasoning else "(No reasoning)"
    code_display = code if code else "(No code)"
    content_display = content if content else "(No content)"
    obs_summary = summarize_observation(observation)
    
    forced_warning = ""
    if is_forced_final:
        forced_warning = "\n⚠️ FORCED FINAL: Budget exhausted, agent forced to submit.\n"
    
    prompt = f"""
**Task**: {task_id} (Repo: {repo})
**Position**: Step {step_position + 1} of {total_steps} | Budget: {budget_pct}% | Remaining: {remaining_steps}
**Final Step**: {"YES" if is_final else "NO"} | Forced: {"YES" if is_forced_final else "NO"}{forced_warning}
**Action**: {action_type} | Tool: {tool_name or "N/A"}

**Recent Context** (last 3 steps):
{context_summary}

**Current Step Reasoning**:
```
{reasoning_display}
```

**Code**:
```
{code_display}
```

**Tool Input**:
```
{content_display}
```

**Result**:
{obs_summary}

---
Evaluate per system prompt. Output ONLY valid JSON.
"""
    
    return prompt.strip()


def extract_json_from_response(response: str) -> Dict:
    """
    Extract JSON from LLM response with automatic repair.
    
    Uses json_repair library to handle malformed JSON gracefully.
    """
    # Try direct parse first
    try:
        return json.loads(response)
    except json.JSONDecodeError:
        pass
    
    # Try extracting from markdown code block
    json_match = re.search(r'```(?:json)?\s*(\{.*?\})\s*```', response, re.DOTALL)
    if json_match:
        try:
            return json.loads(json_match.group(1))
        except json.JSONDecodeError:
            # Try repairing the extracted JSON
            try:
                repaired = repair_json(json_match.group(1))
                logger.info(f"Repaired JSON from markdown block")
                return json.loads(repaired)
            except Exception as e:
                logger.warning(f"Failed to repair JSON from markdown: {e}")
                pass
    
    # Try repairing entire response (avoids O(n²) brace counting)
    try:
        repaired = repair_json(response)
        result = json.loads(repaired)
        if isinstance(result, dict):
            logger.info(f"Repaired JSON from entire response")
            return result
    except Exception as e:
        logger.error(f"All JSON extraction strategies failed: {e}")
        pass
    
    raise json.JSONDecodeError(
        f"No valid JSON found in response: {response[:200]}...",
        response, 0
    )


def validate_axis_output(output: Dict, axis: str) -> Optional[float]:
    """
    Validate and extract score from axis output with fallback handling.
    
    Returns None if score cannot be extracted (not 0.5).
    """
    score_field = f"{axis}_score"
    
    # Try exact field name first
    score = output.get(score_field)
    
    # Try common LLM formatting variations (only unambiguous generic fields)
    if score is None:
        # Only fall back to generic "score" if it's the ONLY score field present
        score_fields_present = [k for k in output.keys() if "score" in k.lower()]
        
        if len(score_fields_present) == 1 and score_fields_present[0] in ["score", "rating"]:
            score = output.get(score_fields_present[0])
            logger.warning(
                f"Axis {axis}: using generic field '{score_fields_present[0]}' (expected '{score_field}'). "
                f"Full output: {str(output)[:300]}"
            )
        elif len(score_fields_present) > 1:
            logger.warning(
                f"Axis {axis}: multiple score fields found {score_fields_present}, cannot disambiguate. "
                f"Full output: {str(output)[:300]}"
            )
            return None
        else:
            logger.warning(
                f"Axis {axis}: no score field found in output. Full output: {str(output)[:300]}"
            )
            return None
    
    if score is None:
        return None
    
    try:
        score = float(score)
        return max(0.0, min(1.0, score))
    except (TypeError, ValueError):
        logger.warning(f"Axis {axis}: could not convert score '{score}' to float")
        return None


# ═══════════════════════════════════════════════════════════════════════════
# MAIN EVALUATION FUNCTION
# ═══════════════════════════════════════════════════════════════════════════

async def evaluate_step(
    step: Dict,
    axis: str,
    client: LLMClient,
    cache: JudgeCache,
    context_steps: Optional[List[Dict]] = None
) -> Optional[float]:
    """
    Evaluate a single step on a specific evaluation axis.
    
    Returns:
        Score (0.0-1.0) or None if evaluation failed
    """
    # Check cache first
    cache_key = compute_cache_key(step, axis, client.config.model, context_steps)
    cached = cache.get(cache_key)
    if cached:
        score = validate_axis_output(cached, axis)
        if score is not None:
            return score
    
    # Get axis config
    axis_config = EVALUATION_AXES[axis]
    
    # Build prompt
    user_prompt = build_user_prompt(step, axis, context_steps)
    
    # Call LLM
    try:
        response, input_tokens, output_tokens = await client.call(
            axis_config["system_prompt"], 
            user_prompt
        )
        
        # Parse response
        output = extract_json_from_response(response)
        
        # Validate and extract score
        score = validate_axis_output(output, axis)
        
        # Only cache if score is valid
        if score is not None:
            cache.set(cache_key, axis, client.config.model, output, raw_response=response)
        
        return score
    
    except Exception as e:
        logger.error(
            f"Axis {axis} evaluation failed for step {step.get('step_position')}: {type(e).__name__}: {str(e)}"
        )
        return None
