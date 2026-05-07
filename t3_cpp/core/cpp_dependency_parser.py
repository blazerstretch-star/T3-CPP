import re
import subprocess
from pathlib import Path

class CppDependencyParser:
    def __init__(self):
        self.include_pattern = r'#include\s*[<"](.*?)[>"]'
        self.function_call_pattern = r'(\w+)\s*\('
    
    def extract_dependencies(self, source_file):
        """Extract C++ dependencies from source file"""
        with open(source_file, 'r') as f:
            content = f.read()
        
        dependencies = {
            'includes': self.extract_includes(content),
            'function_calls': self.extract_function_calls(content),
            'class_usage': self.extract_class_usage(content)
        }
        return dependencies
    
    def extract_dependencies_from_code(self, code_snippet):
        """Extract dependencies from code snippet (for generated code)"""
        dependencies = {
            'includes': self.extract_includes(code_snippet),
            'function_calls': self.extract_function_calls(code_snippet),
            'class_usage': self.extract_class_usage(code_snippet)
        }
        return dependencies
    
    def extract_includes(self, content):
        """Extract #include statements"""
        return re.findall(self.include_pattern, content)
    
    def extract_function_calls(self, content):
        """Extract function calls including std:: functions"""
        # Remove comments and strings first
        cleaned = re.sub(r'//.*?$|/\*.*?\*/', '', content, flags=re.MULTILINE | re.DOTALL)
        cleaned = re.sub(r'".*?"', '', cleaned)
        
        # Find all function calls with various patterns
        all_calls = []
        
        # std:: function calls
        std_calls = re.findall(r'std::(\w+)', cleaned)
        all_calls.extend(std_calls)
        
        # Regular function calls (word followed by parentheses)
        regular_calls = re.findall(r'(\w+)\s*\(', cleaned)
        all_calls.extend(regular_calls)
        
        # Namespace::function calls
        namespace_calls = re.findall(r'(\w+::\w+)\s*\(', cleaned)
        all_calls.extend(namespace_calls)
        
        # Method calls (object.method)
        method_calls = re.findall(r'\.(\w+)\s*\(', cleaned)
        all_calls.extend(method_calls)
        
        # Remove common keywords and operators
        keywords = {'if', 'for', 'while', 'switch', 'return', 'sizeof', 'const_cast', 'static_cast', 'dynamic_cast'}
        all_calls = [call for call in all_calls if call not in keywords]
        
        return list(set(all_calls))  # Remove duplicates
    
    def extract_class_usage(self, content):
        """Extract class instantiations and method calls"""
        class_pattern = r'(\w+)::\w+|(\w+)\s+\w+\s*[({]'
        matches = re.findall(class_pattern, content)
        classes = [m[0] or m[1] for m in matches if m[0] or m[1]]
        return list(set(classes))