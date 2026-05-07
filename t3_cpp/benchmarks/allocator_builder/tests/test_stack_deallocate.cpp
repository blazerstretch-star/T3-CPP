#include "../src/allocator_functions.h"
#include <cassert>
#include <cstring>
#include <vector>

using namespace alb;

// Helper to check if block is last allocated
inline bool is_last_used_block(const char* data, const char* p, const block& b) {
    return (static_cast<char*>(b.ptr) + b.length == p);
}

int main() {
    // ========================================
    // PROPERTY 1: Deallocating null block is no-op
    // ========================================
    auto check_null_dealloc_noop = [](char*& p, const char* data, size_t max_size) {
        char* p_before = p;
        block null_block;
        stack_allocator_impl::deallocate(p, data, max_size, null_block);
        assert(p == p_before && "Deallocating null block must not change stack pointer");
        assert(!null_block && "Null block must remain null after deallocation");
    };
    
    // ========================================
    // PROPERTY 2: Block is invalidated after deallocation
    // ========================================
    auto check_block_invalidated = [](const block& b) {
        assert(!b && "Block must be invalidated (reset) after deallocation");
        assert(b.ptr == nullptr && "Block pointer must be null after deallocation");
        assert(b.length == 0 && "Block length must be zero after deallocation");
    };
    
    // ========================================
    // PROPERTY 3: Last block deallocation reclaims space
    // ========================================
    auto check_last_block_reclaim = [](char* p_after_dealloc, const char* expected_position) {
        assert(p_after_dealloc == expected_position && 
               "Deallocating last block must move stack pointer back to block start");
    };
    
    // ========================================
    // PROPERTY 4: Non-last block deallocation doesn't reclaim
    // ========================================
    auto check_non_last_no_reclaim = [](char* p_before, char* p_after) {
        assert(p_before == p_after && 
               "Deallocating non-last block must not change stack pointer");
    };
    
    // ========================================
    // PROPERTY 5: Deallocating unowned block is safe
    // ========================================
    auto check_unowned_safe = [](char*& p, const char* data, size_t max_size, block& b) {
        char* p_before = p;
        void* ptr_before = b.ptr;
        size_t len_before = b.length;
        stack_allocator_impl::deallocate(p, data, max_size, b);
        // Implementation may or may not invalidate unowned blocks
        // But it must not change the stack pointer
        assert(p == p_before && "Stack pointer must not change for unowned block");
    };
    
    // ========================================
    // PROPERTY 6: Multiple deallocations are idempotent
    // ========================================
    auto check_double_dealloc_safe = [](char*& p, const char* data, size_t max_size, block& b) {
        char* p_after_first = p;
        stack_allocator_impl::deallocate(p, data, max_size, b);
        char* p_after_second = p;
        assert(p_after_first == p_after_second && 
               "Double deallocation must not change state further");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Deallocate null block
    char buffer1[1024];
    char* p1 = buffer1 + 100;
    check_null_dealloc_noop(p1, buffer1, 1024);
    
    // Test 2: Deallocate last block (should reclaim)
    char buffer2[1024];
    char* p2 = buffer2;
    block b2 = stack_allocator_impl::allocate(p2, buffer2, 1024, 16, 64);
    char* block_start = static_cast<char*>(b2.ptr);
    assert(b2 && "Allocation must succeed");
    stack_allocator_impl::deallocate(p2, buffer2, 1024, b2);
    check_block_invalidated(b2);
    check_last_block_reclaim(p2, block_start);
    
    // Test 3: Deallocate non-last block (no reclaim)
    char buffer3[1024];
    char* p3 = buffer3;
    block b3_1 = stack_allocator_impl::allocate(p3, buffer3, 1024, 16, 64);
    block b3_2 = stack_allocator_impl::allocate(p3, buffer3, 1024, 16, 64);
    char* p3_before = p3;
    stack_allocator_impl::deallocate(p3, buffer3, 1024, b3_1);
    check_block_invalidated(b3_1);
    check_non_last_no_reclaim(p3_before, p3);
    
    // Test 4: Deallocate in LIFO order (stack behavior)
    char buffer4[1024];
    char* p4 = buffer4;
    block b4_1 = stack_allocator_impl::allocate(p4, buffer4, 1024, 16, 32);
    block b4_2 = stack_allocator_impl::allocate(p4, buffer4, 1024, 16, 48);
    block b4_3 = stack_allocator_impl::allocate(p4, buffer4, 1024, 16, 64);
    
    char* b3_start = static_cast<char*>(b4_3.ptr);
    stack_allocator_impl::deallocate(p4, buffer4, 1024, b4_3);
    check_last_block_reclaim(p4, b3_start);
    
    char* b2_start = static_cast<char*>(b4_2.ptr);
    stack_allocator_impl::deallocate(p4, buffer4, 1024, b4_2);
    check_last_block_reclaim(p4, b2_start);
    
    char* b1_start = static_cast<char*>(b4_1.ptr);
    stack_allocator_impl::deallocate(p4, buffer4, 1024, b4_1);
    check_last_block_reclaim(p4, b1_start);
    
    assert(p4 == buffer4 && "After deallocating all blocks, pointer should return to start");
    
    // Test 5: Deallocate unowned block (from different buffer)
    char buffer5[1024];
    char other_buffer[1024];
    char* p5 = buffer5 + 100;
    block b5_unowned{other_buffer + 50, 64};
    check_unowned_safe(p5, buffer5, 1024, b5_unowned);
    
    // Test 6: Double deallocation safety
    char buffer6[1024];
    char* p6 = buffer6;
    block b6 = stack_allocator_impl::allocate(p6, buffer6, 1024, 16, 64);
    stack_allocator_impl::deallocate(p6, buffer6, 1024, b6);
    check_double_dealloc_safe(p6, buffer6, 1024, b6);
    
    // Test 7: Deallocate all blocks in random order
    char buffer7[1024];
    char* p7 = buffer7;
    std::vector<block> blocks7;
    for (int i = 0; i < 5; ++i) {
        blocks7.push_back(stack_allocator_impl::allocate(p7, buffer7, 1024, 16, 32));
    }
    
    // Deallocate in order: 4 (last), 1 (middle), 2 (middle), 0 (first), 3 (middle)
    char* p7_before_4 = p7;
    stack_allocator_impl::deallocate(p7, buffer7, 1024, blocks7[4]);
    check_block_invalidated(blocks7[4]);
    assert(p7 < p7_before_4 && "Deallocating last block must reclaim space");
    
    char* p7_before_1 = p7;
    stack_allocator_impl::deallocate(p7, buffer7, 1024, blocks7[1]);
    check_block_invalidated(blocks7[1]);
    check_non_last_no_reclaim(p7_before_1, p7);
    
    // Test 8: Allocate-deallocate-allocate pattern
    char buffer8[1024];
    char* p8 = buffer8;
    block b8_1 = stack_allocator_impl::allocate(p8, buffer8, 1024, 16, 64);
    char* after_first_alloc = p8;
    stack_allocator_impl::deallocate(p8, buffer8, 1024, b8_1);
    assert(p8 == buffer8 && "After dealloc, pointer should be at start");
    
    block b8_2 = stack_allocator_impl::allocate(p8, buffer8, 1024, 16, 64);
    assert(p8 == after_first_alloc && "Re-allocation should reach same position");
    
    // Test 9: Deallocate with zero-length block
    char buffer9[1024];
    char* p9 = buffer9 + 100;
    block b9_zero{buffer9 + 50, 0};
    if (b9_zero) {  // Only test if zero-length blocks are considered valid
        stack_allocator_impl::deallocate(p9, buffer9, 1024, b9_zero);
        check_block_invalidated(b9_zero);
    }
    
    // Test 10: Stress test - many allocations and deallocations
    char buffer10[4096];
    char* p10 = buffer10;
    std::vector<block> blocks10;
    
    // Allocate 64 blocks
    for (int i = 0; i < 64; ++i) {
        blocks10.push_back(stack_allocator_impl::allocate(p10, buffer10, 4096, 16, 64));
    }
    
    // Deallocate in reverse order (LIFO)
    for (int i = 63; i >= 0; --i) {
        stack_allocator_impl::deallocate(p10, buffer10, 4096, blocks10[i]);
        check_block_invalidated(blocks10[i]);
    }
    
    assert(p10 == buffer10 && "After deallocating all blocks in LIFO order, pointer should be at start");
    
    return 0;
}
