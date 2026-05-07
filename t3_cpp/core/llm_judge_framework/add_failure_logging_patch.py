#!/usr/bin/env python3
"""
Patch to add comprehensive failure logging to add_llm_judge.py
Run this to add failure tracking print statements
"""

import sys
from pathlib import Path

def add_failure_logging():
    """Add failure logging statements to add_llm_judge.py"""
    
    script_path = Path(__file__).parent / "add_llm_judge.py"
    
    if not script_path.exists():
        print(f"ERROR: {script_path} not found")
        return False
    
    with open(script_path, 'r') as f:
        content = f.read()
    
    # Check if already patched
    if "FAILURE_TRACKING_PATCHED" in content:
        print("✓ Script already patched with failure logging")
        return True
    
    # Add failure tracking after imports
    import_section = 'logger = logging.getLogger("add_llm_judge")'
    
    failure_tracking = '''
# FAILURE_TRACKING_PATCHED
# Failure tracking counters
failure_stats = {
    "api_errors": 0,
    "json_parse_errors": 0,
    "axis_evaluation_failures": 0,
    "cache_errors": 0,
    "trajectory_failures": 0,
    "step_failures": 0,
}

def log_failure(failure_type: str, details: str, trajectory_id: str = None, step_pos: int = None):
    """Log failure with context"""
    failure_stats[failure_type] = failure_stats.get(failure_type, 0) + 1
    context = f"[Traj: {trajectory_id}, Step: {step_pos}]" if trajectory_id else ""
    print(f"\\n⚠️  FAILURE {context}: {failure_type}")
    print(f"   Details: {details}")
    logger.error(f"{failure_type} {context}: {details}")

def print_failure_summary():
    """Print summary of all failures"""
    print("\\n" + "=" * 60)
    print("  FAILURE SUMMARY")
    print("=" * 60)
    total_failures = sum(failure_stats.values())
    if total_failures == 0:
        print("✓ No failures detected")
    else:
        print(f"Total failures: {total_failures}")
        for failure_type, count in sorted(failure_stats.items()):
            if count > 0:
                print(f"  {failure_type}: {count}")
    print("=" * 60 + "\\n")
'''
    
    content = content.replace(import_section, import_section + failure_tracking)
    
    # Add failure logging to process_single_step
    old_axis_error = '''worker_logger.error(f"    Step {step_idx+1} Axis {axis} failed: {type(result).__name__}")
                                    axis_failures[axis] = f"{type(result).__name__}: {str(result)[:100]}"'''
    
    new_axis_error = '''worker_logger.error(f"    Step {step_idx+1} Axis {axis} failed: {type(result).__name__}")
                                    axis_failures[axis] = f"{type(result).__name__}: {str(result)[:100]}"
                                    log_failure("axis_evaluation_failures", 
                                              f"Axis {axis}: {type(result).__name__}: {str(result)[:100]}",
                                              step.get("trajectory_id"), step_idx)'''
    
    content = content.replace(old_axis_error, new_axis_error)
    
    # Add failure logging to trajectory processing
    old_traj_error = '''worker_logger.error(f"Failed trajectory {traj_id}: {type(e).__name__}: {str(e)}")'''
    
    new_traj_error = '''worker_logger.error(f"Failed trajectory {traj_id}: {type(e).__name__}: {str(e)}")
                        log_failure("trajectory_failures", f"{type(e).__name__}: {str(e)}", traj_id)
                        print(f"\\n❌ Trajectory {traj_id} FAILED: {type(e).__name__}")'''
    
    content = content.replace(old_traj_error, new_traj_error)
    
    # Add failure summary to main_async
    old_done = '''cache.close()
    logger.info("Done!")'''
    
    new_done = '''cache.close()
    print_failure_summary()
    logger.info("Done!")'''
    
    content = content.replace(old_done, new_done)
    
    # Write patched version
    backup_path = script_path.with_suffix('.py.backup')
    script_path.rename(backup_path)
    print(f"✓ Created backup: {backup_path}")
    
    with open(script_path, 'w') as f:
        f.write(content)
    
    print(f"✓ Patched {script_path} with failure logging")
    return True

if __name__ == "__main__":
    success = add_failure_logging()
    sys.exit(0 if success else 1)
