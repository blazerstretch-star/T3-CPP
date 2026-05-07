#!/usr/bin/env python3
"""
Gold-Standard Trajectory Builder for PRM Dataset
Captures full agent execution trace with no truncation
"""

import json
import time
from pathlib import Path
from typing import Dict, List, Any, Optional


class TrajectoryBuilder:
    """Builds research-grade trajectory dataset for PRM training"""
    
    def __init__(self, task_id: str, repo: str, model: str, budget: Dict[str, Any], beam_id: int = 0, temperature: float = 0.0):
        self.task_id = task_id
        self.repo = repo
        self.model = model
        self.budget = budget
        self.beam_id = beam_id
        self.temperature = temperature
        
        self.trajectory = []
        self.start_time = time.time()
        self.final_submission = None
        self.metrics = {}
        # Optional test artifacts produced during generation (filled when available)
        self.test_results: Optional[Dict[str, Any]] = None
        self.passed_tests: Optional[bool] = None
        self.test_stats: Optional[Dict[str, Any]] = None
        self.code_similarity: Optional[Dict[str, Any]] = None
        self.test_failures: Optional[list] = None
        
    def add_step(self, 
                 step_id: int,
                 state: Dict[str, Any],
                 reasoning: str,
                 action: Optional[Dict[str, Any]],
                 result: Optional[Any],
                 api_status: str = "ok",
                 api_error: Optional[str] = None,
                 api_duration: float = 0.0,
                 tokens_used: int = 0,
                 token_breakdown: Optional[Dict[str, int]] = None,
                 reasoning_content: Optional[str] = None,
                 is_thinking_model: bool = False):
        """Add a single trajectory step (no truncation)"""
        
        # Determine reasoning_source
        if reasoning_content:
            reasoning_source = "thinking_model"
        elif not is_thinking_model:
            reasoning_source = "plain_text"
        elif reasoning and "[auto-reconstructed]" not in reasoning:
            reasoning_source = "think_tags"
        else:
            reasoning_source = "auto_reconstructed"

        step = {
            "step_id": step_id,
            "state": {
                "iteration": state.get("iteration", 0),
                "tool_calls_so_far": state.get("tool_calls", 0),
                "tokens_so_far": state.get("tokens", 0),
                "elapsed_time": time.time() - self.start_time
            },
            "reasoning": reasoning,
            "reasoning_content": reasoning_content,  # Raw reasoning_content from thinking model
            "reasoning_source": reasoning_source,
            "action": action,  # {"tool": "search_repo_code", "args": {...}}
            "result": result,  # Full tool output, no truncation
            "api_status": api_status,
            "api_error": api_error,
            "api_duration_sec": api_duration,
            "tokens_used": tokens_used,
            "token_breakdown": token_breakdown or {},  # {completion, prompt, total, reasoning_tokens}
            "is_thinking_model": is_thinking_model
        }
        
        self.trajectory.append(step)
    
    def set_final_submission(self,
                            code: str,
                            compiled: bool,
                            compile_error: Optional[str] = None,
                            failure_category: Optional[str] = None,
                            compile_attempts: int = 0,
                            last_compile_errors: Optional[List[str]] = None,
                            ast_score: Optional[float] = None):
        """Set final submission outcome (test results added by evaluation)
        
        failure_category values:
          - "success": compiled and submitted within budget
          - "compile_failure": code extracted but failed to compile
          - "budget_exhausted": budget ran out, forced answer does NOT compile
          - "budget_exhausted_compiles": budget ran out, but forced answer DOES compile
          - "no_code_extracted": forced answer produced no extractable code
          - "logic_error": compiled but tests failed (set by evaluation)
        
        ast_score: tree-sitter AST quality (0.0–1.0), ratio of non-error nodes.
                   Higher = more syntactically valid C++. Useful PRM signal.
        """
        
        self.final_submission = {
            "code": code,
            "compiled": compiled,
            "compile_error": compile_error,
            "failure_category": failure_category,
            "compile_attempts": compile_attempts,
            "last_compile_errors": last_compile_errors or [],
            "ast_score": ast_score
        }
    
    def set_metrics(self,
                   total_steps: int,
                   total_tool_calls: int,
                   tokens_used: int,
                   wall_clock_sec: float,
                   stop_reason: str):
        """Set final metrics"""
        
        self.metrics = {
            "total_steps": total_steps,
            "total_tool_calls": total_tool_calls,
            "tokens_used": tokens_used,
            "wall_clock_sec": wall_clock_sec,
            "stop_reason": stop_reason
        }
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to gold-standard format"""
        base = {
            "repo": self.repo,
            "task_id": self.task_id,
            "model": self.model,
            "beam_id": self.beam_id,
            "temperature": self.temperature,
            "budget": self.budget,
            "trajectory": self.trajectory,
            "final_submission": self.final_submission,
            "metrics": self.metrics
        }

        # Attach test artifacts if present (produced during generation or evaluation)
        if self.test_results is not None:
            base['test_results'] = self.test_results
        if self.passed_tests is not None:
            base['passed_tests'] = self.passed_tests
        if self.test_stats is not None:
            base['test_stats'] = self.test_stats
        if getattr(self, 'code_similarity', None) is not None:
            base['code_similarity'] = self.code_similarity
        if getattr(self, 'test_failures', None) is not None:
            base['test_failures'] = self.test_failures
        # Conversation snapshots removed from persisted trajectory

        return base
    
    def save(self, output_path: Path):
        """Save trajectory to JSON file"""
        
        output_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(output_path, 'w') as f:
            json.dump(self.to_dict(), f, indent=2, default=str)
    
    @staticmethod
    def create_output_path(base_dir: Path, 
                          repo: str, 
                          task_id: str, 
                          model: str, 
                          beam_id: int = 1,
                          setting: str = "") -> Path:
        """Create standardized output path"""
        
        # Format: results/{model}/{repo}/{setting}/{task_id}_beam_{beam_id}_log_trj.json
        # Keep model path as-is (e.g., qwen/qwen3-next-80b-a3b-instruct)
        safe_task = task_id.replace('::', '_').replace('/', '_')
        
        if setting:
            return base_dir / model / repo / setting / f"{safe_task}_beam_{beam_id}_log_trj.json"
        else:
            return base_dir / model / repo / f"{safe_task}_beam_{beam_id}_log_trj.json"