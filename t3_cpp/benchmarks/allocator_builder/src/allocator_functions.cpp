#include "allocator_functions.h"
#include <cstring>
#include <algorithm>
#include <cstdint>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace alb {

namespace stack_allocator_impl {

bool owns(const void* data, size_t max_size, const block& b) {
    // FUNCTION_ID: allocator_func001 - START
    if (!b) return false;
    const char* data_start = static_cast<const char*>(data);
    const char* data_end = data_start + max_size;
    const char* block_ptr = static_cast<const char*>(b.ptr);
    return (block_ptr >= data_start && block_ptr < data_end);
    // FUNCTION_ID: allocator_func001 - END
}

block allocate(char*& p, const char* data, size_t max_size, size_t alignment, size_t n) {
    // FUNCTION_ID: allocator_func002 - START
    block result;
    if (n == 0) return result;
    
    // Align the pointer first
    auto ptr_val = reinterpret_cast<uintptr_t>(p);
    auto aligned_ptr_val = (ptr_val + alignment - 1) & ~(alignment - 1);
    auto aligned_ptr = reinterpret_cast<char*>(aligned_ptr_val);
    
    // Inline helper: round_to_alignment
    const auto aligned_length = (n + alignment - 1) & ~(alignment - 1);
    if (aligned_ptr + aligned_length > data + max_size) {
        return result;
    }
    
    result.ptr = aligned_ptr;
    result.length = aligned_length;
    p = aligned_ptr + aligned_length;
    return result;
    // FUNCTION_ID: allocator_func002 - END
}

void deallocate(char*& p, const char* data, size_t max_size, block& b) {
    // FUNCTION_ID: allocator_func003 - START
    if (!b) return;
    if (!owns(data, max_size, b)) return;
    
    // Inline helper: is_last_used_block
    if (static_cast<char*>(b.ptr) + b.length == p) {
        p = static_cast<char*>(b.ptr);
    }
    b.reset();
    // FUNCTION_ID: allocator_func003 - END
}

bool reallocate(char*& p, const char* data, size_t max_size, size_t alignment, block& b, size_t n) {
    // FUNCTION_ID: allocator_func004 - START
    if (b.length == n) return true;
    if (n == 0) {
        deallocate(p, data, max_size, b);
        return true;
    }
    if (!b) {
        b = allocate(p, data, max_size, alignment, n);
        return static_cast<bool>(b);
    }
    
    // Inline helper: round_to_alignment
    const auto aligned_length = (n + alignment - 1) & ~(alignment - 1);
    
    // Inline helper: is_last_used_block
    if (static_cast<char*>(b.ptr) + b.length == p) {
        if (static_cast<char*>(b.ptr) + aligned_length <= data + max_size) {
            b.length = aligned_length;
            p = static_cast<char*>(b.ptr) + aligned_length;
            return true;
        }
        return false;
    }
    
    if (b.length > n) {
        // Inline helper: round_to_alignment
        b.length = (n + alignment - 1) & ~(alignment - 1);
        return true;
    }
    
    auto new_block = allocate(p, data, max_size, alignment, aligned_length);
    if (new_block) {
        std::memcpy(new_block.ptr, b.ptr, b.length);
        b = new_block;
        return true;
    }
    return false;
    // FUNCTION_ID: allocator_func004 - END
}

} // namespace stack_allocator_impl
} // namespace alb
