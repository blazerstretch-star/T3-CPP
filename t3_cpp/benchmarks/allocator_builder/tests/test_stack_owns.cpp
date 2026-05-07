#include "../src/allocator_functions.h"
#include <cassert>
#include <cstdint>
#include <initializer_list>

using namespace alb;

int main() {
    // ========================================
    // PROPERTY 1: Null block never owned
    // ========================================
    auto check_null_block_not_owned = [](const void* data, size_t max_size) {
        block null_block;
        assert(!null_block && "Null block should have false bool conversion");
        assert(!stack_allocator_impl::owns(data, max_size, null_block) && 
               "Null block must never be owned by any allocator");
    };
    
    // ========================================
    // PROPERTY 2: Block inside range is owned
    // ========================================
    auto check_inside_range_owned = [](const void* data, size_t max_size, const block& b) {
        const char* data_start = static_cast<const char*>(data);
        const char* data_end = data_start + max_size;
        const char* block_ptr = static_cast<const char*>(b.ptr);
        
        bool ptr_in_range = (block_ptr >= data_start && block_ptr < data_end);
        bool is_owned = stack_allocator_impl::owns(data, max_size, b);
        
        if (b && ptr_in_range) {
            assert(is_owned && "Block with pointer inside range must be owned");
        }
    };
    
    // ========================================
    // PROPERTY 3: Block outside range not owned
    // ========================================
    auto check_outside_range_not_owned = [](const void* data, size_t max_size, const block& b) {
        const char* data_start = static_cast<const char*>(data);
        const char* data_end = data_start + max_size;
        const char* block_ptr = static_cast<const char*>(b.ptr);
        
        bool ptr_before_range = (block_ptr < data_start);
        bool ptr_after_range = (block_ptr >= data_end);
        bool is_owned = stack_allocator_impl::owns(data, max_size, b);
        
        if (b && (ptr_before_range || ptr_after_range)) {
            assert(!is_owned && "Block with pointer outside range must not be owned");
        }
    };
    
    // ========================================
    // PROPERTY 4: Ownership is deterministic
    // ========================================
    auto check_deterministic = [](const void* data, size_t max_size, const block& b) {
        bool result1 = stack_allocator_impl::owns(data, max_size, b);
        bool result2 = stack_allocator_impl::owns(data, max_size, b);
        assert(result1 == result2 && "Ownership check must be deterministic");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Null block with various buffer sizes
    char buffer1[1024];
    check_null_block_not_owned(buffer1, 1024);
    check_null_block_not_owned(buffer1, 1);
    check_null_block_not_owned(buffer1, 10000);
    
    // Test 2: Block at buffer start
    block b_start{buffer1, 64};
    check_inside_range_owned(buffer1, 1024, b_start);
    check_deterministic(buffer1, 1024, b_start);
    assert(stack_allocator_impl::owns(buffer1, 1024, b_start) && 
           "Block at buffer start must be owned");
    
    // Test 3: Block in middle of buffer
    block b_middle{buffer1 + 512, 64};
    check_inside_range_owned(buffer1, 1024, b_middle);
    check_deterministic(buffer1, 1024, b_middle);
    assert(stack_allocator_impl::owns(buffer1, 1024, b_middle) && 
           "Block in middle of buffer must be owned");
    
    // Test 4: Block at last valid position (just before end)
    block b_near_end{buffer1 + 1023, 1};
    check_inside_range_owned(buffer1, 1024, b_near_end);
    assert(stack_allocator_impl::owns(buffer1, 1024, b_near_end) && 
           "Block at last valid position must be owned");
    
    // Test 5: Block exactly at end boundary (not owned)
    block b_at_end{buffer1 + 1024, 64};
    check_outside_range_not_owned(buffer1, 1024, b_at_end);
    assert(!stack_allocator_impl::owns(buffer1, 1024, b_at_end) && 
           "Block at end boundary must not be owned");
    
    // Test 6: Block far outside buffer (after)
    block b_far_after{buffer1 + 5000, 64};
    check_outside_range_not_owned(buffer1, 1024, b_far_after);
    assert(!stack_allocator_impl::owns(buffer1, 1024, b_far_after) && 
           "Block far after buffer must not be owned");
    
    // Test 7: Block before buffer (using separate memory)
    char other_buffer[100];
    block b_before{other_buffer, 64};
    check_outside_range_not_owned(buffer1, 1024, b_before);
    assert(!stack_allocator_impl::owns(buffer1, 1024, b_before) && 
           "Block from different memory must not be owned");
    
    // Test 8: Zero-size buffer edge case
    char tiny_buffer[1];
    block b_tiny{tiny_buffer, 1};
    assert(stack_allocator_impl::owns(tiny_buffer, 1, b_tiny) && 
           "Block in single-byte buffer must be owned");
    check_null_block_not_owned(tiny_buffer, 1);
    
    // Test 9: Large buffer with block at various positions
    char large_buffer[10000];
    for (size_t offset : {0, 1, 100, 5000, 9999}) {
        block b_large{large_buffer + offset, 1};
        check_inside_range_owned(large_buffer, 10000, b_large);
        assert(stack_allocator_impl::owns(large_buffer, 10000, b_large) && 
               "Block at any valid offset in large buffer must be owned");
    }
    
    // Test 10: Block with zero length (but valid pointer)
    block b_zero_len{buffer1 + 100, 0};
    if (b_zero_len) {  // If implementation considers zero-length blocks valid
        check_inside_range_owned(buffer1, 1024, b_zero_len);
    }
    
    return 0;
}
