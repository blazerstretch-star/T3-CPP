"""
Agentic Stopping Strategies for t3_cpp
Provides 4 strategies to control agent execution budgets
"""

import time
from abc import ABC, abstractmethod
from typing import Dict, Any


class AgenticStrategy(ABC):
    """Base class for stopping strategies"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.metrics = {
            'iterations': 0,
            'tool_calls': 0,
            'tokens': 0,
            'start_time': None,
            'stop_reason': None,
            'tools_current_iteration': 0
        }
    
    @abstractmethod
    def should_stop(self) -> bool:
        """Check if agent should stop"""
        pass
    
    @abstractmethod
    def can_use_tool(self) -> bool:
        """Check if agent can use more tools"""
        pass
    
    def start(self):
        """Start timing"""
        self.metrics['start_time'] = time.time()
    
    def next_iteration(self):
        """Increment iteration counter"""
        self.metrics['iterations'] += 1
        self.metrics['tools_current_iteration'] = 0
    
    def add_tool_call(self):
        """Increment tool call counters"""
        self.metrics['tool_calls'] += 1
        self.metrics['tools_current_iteration'] += 1
    
    def add_tokens(self, count: int):
        """Add token usage"""
        self.metrics['tokens'] += count
    
    def get_metrics(self) -> Dict[str, Any]:
        """Get current metrics"""
        if self.metrics['start_time']:
            self.metrics['elapsed_time'] = time.time() - self.metrics['start_time']
        return self.metrics.copy()


class IterationStrategy(AgenticStrategy):
    """Stop based on iteration count only"""
    
    def should_stop(self) -> bool:
        if self.metrics['iterations'] >= self.config['max_iterations']:
            self.metrics['stop_reason'] = 'max_iterations'
            return True
        return False
    
    def can_use_tool(self) -> bool:
        # Check total tool calls
        if self.metrics['tool_calls'] >= self.config['max_tool_calls']:
            return False
        
        # Check per-iteration limit
        per_iter_limit = self.config.get('tool_call_limit_per_iteration', 3)
        if self.metrics['tools_current_iteration'] >= per_iter_limit:
            return False
        
        return True


class TokenStrategy(AgenticStrategy):
    """Stop based on token budget"""
    
    def should_stop(self) -> bool:
        if self.metrics['tokens'] >= self.config['max_tokens']:
            self.metrics['stop_reason'] = 'max_tokens'
            return True
        return False
    
    def can_use_tool(self) -> bool:
        # Estimate tool call cost (200 tokens per call)
        estimated_cost = 200
        if self.metrics['tokens'] + estimated_cost >= self.config['max_tokens']:
            return False
        
        # Check per-iteration limit
        per_iter_limit = self.config.get('tool_call_limit_per_iteration', 3)
        if self.metrics['tools_current_iteration'] >= per_iter_limit:
            return False
        
        return True


class TimeStrategy(AgenticStrategy):
    """Stop based on time limit"""
    
    def should_stop(self) -> bool:
        if self.metrics['start_time'] is None:
            return False
        
        elapsed = time.time() - self.metrics['start_time']
        if elapsed >= self.config['max_time']:
            self.metrics['stop_reason'] = 'timeout'
            return True
        return False
    
    def can_use_tool(self) -> bool:
        # Estimate tool call time (10 seconds per call)
        if self.metrics['start_time']:
            elapsed = time.time() - self.metrics['start_time']
            estimated_time = 10
            if elapsed + estimated_time >= self.config['max_time']:
                return False
        
        # Check per-iteration limit
        per_iter_limit = self.config.get('tool_call_limit_per_iteration', 3)
        if self.metrics['tools_current_iteration'] >= per_iter_limit:
            return False
        
        return True


class HybridStrategy(AgenticStrategy):
    """Stop when ANY limit is reached (most restrictive)"""
    
    def should_stop(self) -> bool:
        # Check iteration limit
        if self.metrics['iterations'] >= self.config['max_iterations']:
            self.metrics['stop_reason'] = 'max_iterations'
            return True
        
        # Check tool call limit
        if self.metrics['tool_calls'] >= self.config['max_tool_calls']:
            self.metrics['stop_reason'] = 'max_tool_calls'
            return True
        
        # Check token limit
        if self.metrics['tokens'] >= self.config['max_tokens']:
            self.metrics['stop_reason'] = 'max_tokens'
            return True
        
        # Check time limit
        if self.metrics['start_time']:
            elapsed = time.time() - self.metrics['start_time']
            if elapsed >= self.config['max_time']:
                self.metrics['stop_reason'] = 'timeout'
                return True
        
        return False
    
    def can_use_tool(self) -> bool:
        # Check total tool calls
        if self.metrics['tool_calls'] >= self.config['max_tool_calls']:
            return False
        
        # Check per-iteration limit
        per_iter_limit = self.config.get('tool_call_limit_per_iteration', 3)
        if self.metrics['tools_current_iteration'] >= per_iter_limit:
            return False
        
        # Check token budget (estimate 200 tokens per tool call)
        if self.metrics['tokens'] + 200 >= self.config['max_tokens']:
            return False
        
        # Check time budget (estimate 10 seconds per tool call)
        if self.metrics['start_time']:
            elapsed = time.time() - self.metrics['start_time']
            if elapsed + 10 >= self.config['max_time']:
                return False
        
        return True


def get_strategy(strategy_name: str, config: Dict[str, Any]) -> AgenticStrategy:
    """
    Factory function to get strategy instance
    
    Args:
        strategy_name: One of 'iteration', 'token', 'time', 'hybrid'
        config: Configuration dict with budget limits
    
    Returns:
        Strategy instance
    """
    strategies = {
        'iteration': IterationStrategy,
        'token': TokenStrategy,
        'time': TimeStrategy,
        'hybrid': HybridStrategy
    }
    
    if strategy_name not in strategies:
        raise ValueError(f"Unknown strategy: {strategy_name}. Choose from {list(strategies.keys())}")
    
    return strategies[strategy_name](config)
