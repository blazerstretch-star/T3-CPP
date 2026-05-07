#include "../src/allocator_functions.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace alb;

// Helper to round up to alignment
inline size_t round_to_alignment(size_t alignment, size_t n) {
    return (n + alignment - 1) & ~(alignment - 1);
}

// Helper to check if block is last allocated
inline bool is_last_used_block(const char* data, const char* p, const block& b) {
    return (static_cast<char*>(b.ptr) + b.length == p);
}

int main() {
    // ========================================
    // PROPERTY 1: Same-size reallocation succeeds without moving
    // ========================================
    auto check_same_size_success = [](char* p_before, char* p_after, const block& b_before, 
                                       const block& b_after, size_t requested) {
        assert(b_after && "Same-size reallocation must succeed");
        assert(b_after.ptr == b_before.ptr && "Same-size reallocation must not move block");
        assert(b_after.length == b_before.length && "Same-size reallocation must keep length");
        assert(p_after == p_before && "Same-size reallocation must not move stack pointer");
    };
    
    // ========================================
    // PROPERTY 2: Zero-size reallocation deallocates
    // ========================================
    auto check_zero_size_deallocates = [](const block& b, bool result) {
        assert(result && "Zero-size reallocation must succeed");
        assert(!b && "Zero-size reallocation must invalidate block");
        assert(b.ptr == nullptr && "Zero-size reallocation must null pointer");
        assert(b.length == 0 && "Zero-size reallocation must zero length");
    };
    
    // ========================================
    // PROPERTY 3: Null block reallocation allocates new
    // ========================================
    auto check_null_allocates = [](const block& b, size_t requested, size_t alignment, bool result) {
        if (result) {
            assert(b && "Successful null-block reallocation must return valid block");
            assert(b.ptr != nullptr && "Allocated block must have non-null pointer");
            assert(b.length >= requested && "Allocated block must have sufficient length");
            assert(b.length == round_to_alignment(alignment, requested) && 
                   "Allocated block must be aligned");
        } else {
            assert(!b && "Failed null-block reallocation must keep block null");
        }
    };
    
    // ========================================
    // PROPERTY 4: Data preservation on reallocation
    // ========================================
    auto check_data_preserved = [](const block& b_before, const block& b_after, 
                                    const char* test_data, size_t data_len) {
        if (b_after && b_before) {
            size_t copy_len = std::min(b_before.length, data_len);
            assert(std::memcmp(b_after.ptr, test_data, copy_len) == 0 && 
                   "Reallocation must preserve existing data");
        }
    };
    
    // ========================================
    // PROPERTY 5: Shrinking always succeeds
    // ========================================
    auto check_shrink_succeeds = [](bool result, const block& b, size_t new_size, size_t alignment) {
        assert(result && "Shrinking reallocation must always succeed");
        assert(b && "Shrunk block must remain valid");
        assert(b.length >= new_size && "Shrunk block must accommodate new size");
        assert(b.length == round_to_alignment(alignment, new_size) && 
               "Shrunk block must be aligned");
    };
    
    // ========================================
    // PROPERTY 6: Last block expansion in-place when space available
    // ========================================
    auto check_last_block_expansion = [](const char* data, const char* p, const block& b_before, 
                                          const block& b_after, size_t new_size, size_t max_size, 
                                          size_t alignment, bool result) {
        if (is_last_used_block(data, p, b_before)) {
            size_t needed = round_to_alignment(alignment, new_size);
            size_t available = (data + max_size) - static_cast<char*>(b_before.ptr);
            
            if (needed <= available) {
                assert(result && "Last block expansion with space must succeed");
                assert(b_after.ptr == b_before.ptr && "Last block expansion must not move");
                assert(b_after.length == needed && "Last block must expand to new size");
            }
        }
    };
    
    // ========================================
    // PROPERTY 7: Reallocation maintains length alignment
    // ========================================
    auto check_alignment_maintained = [](const block& b, size_t alignment, const char* buffer_start) {
        if (b) {
            // Length must always be aligned
            assert(b.length % alignment == 0 && "Reallocated length must be aligned");
        }
    };
    
    // ========================================
    // PROPERTY 8: Failed reallocation preserves original block
    // ========================================
    auto check_failure_preserves = [](const block& b_before, const block& b_after, bool result) {
        if (!result) {
            assert(b_after.ptr == b_before.ptr && "Failed reallocation must preserve pointer");
            assert(b_after.length == b_before.length && "Failed reallocation must preserve length");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Same-size reallocation
    char buffer1[1024];
    char* p1 = buffer1;
    block b1 = stack_allocator_impl::allocate(p1, buffer1, 1024, 16, 64);
    block b1_copy = b1;
    char* p1_before = p1;
    bool r1 = stack_allocator_impl::reallocate(p1, buffer1, 1024, 16, b1, 64);
    check_same_size_success(p1_before, p1, b1_copy, b1, 64);
    
    // Test 2: Zero-size reallocation (deallocation)
    char buffer2[1024];
    char* p2 = buffer2;
    block b2 = stack_allocator_impl::allocate(p2, buffer2, 1024, 16, 64);
    bool r2 = stack_allocator_impl::reallocate(p2, buffer2, 1024, 16, b2, 0);
    check_zero_size_deallocates(b2, r2);
    
    // Test 3: Null block reallocation (allocation)
    alignas(16) char buffer3[1024];
    char* p3 = buffer3;
    block b3;
    bool r3 = stack_allocator_impl::reallocate(p3, buffer3, 1024, 16, b3, 64);
    check_null_allocates(b3, 64, 16, r3);
    check_alignment_maintained(b3, 16, buffer3);
    
    // Test 4: Expand last block in-place
    alignas(16) char buffer4[1024];
    char* p4 = buffer4;
    block b4 = stack_allocator_impl::allocate(p4, buffer4, 1024, 16, 64);
    void* original_ptr = b4.ptr;
    bool r4 = stack_allocator_impl::reallocate(p4, buffer4, 1024, 16, b4, 128);
    check_last_block_expansion(buffer4, p4, {original_ptr, 64}, b4, 128, 1024, 16, r4);
    assert(r4 && "Expanding last block with space must succeed");
    assert(b4.ptr == original_ptr && "Last block expansion must not move");
    check_alignment_maintained(b4, 16, buffer4);
    
    // Test 5: Shrink last block
    alignas(16) char buffer5[1024];
    char* p5 = buffer5;
    block b5 = stack_allocator_impl::allocate(p5, buffer5, 1024, 16, 128);
    void* ptr5 = b5.ptr;
    bool r5 = stack_allocator_impl::reallocate(p5, buffer5, 1024, 16, b5, 64);
    check_shrink_succeeds(r5, b5, 64, 16);
    assert(b5.ptr == ptr5 && "Shrinking must not move block");
    check_alignment_maintained(b5, 16, buffer5);
    
    // Test 6: Shrink non-last block
    char buffer6[1024];
    char* p6 = buffer6;
    block b6_1 = stack_allocator_impl::allocate(p6, buffer6, 1024, 16, 128);
    block b6_2 = stack_allocator_impl::allocate(p6, buffer6, 1024, 16, 64);
    void* ptr6 = b6_1.ptr;
    bool r6 = stack_allocator_impl::reallocate(p6, buffer6, 1024, 16, b6_1, 64);
    check_shrink_succeeds(r6, b6_1, 64, 16);
    assert(b6_1.ptr == ptr6 && "Shrinking non-last block must not move");
    
    // Test 7: Expand non-last block (allocate new + copy)
    alignas(16) char buffer7[1024];
    char* p7 = buffer7;
    block b7_1 = stack_allocator_impl::allocate(p7, buffer7, 1024, 16, 64);
    const char test_data[] = "test_data_12345";
    std::memcpy(b7_1.ptr, test_data, sizeof(test_data));
    block b7_2 = stack_allocator_impl::allocate(p7, buffer7, 1024, 16, 64);
    
    block b7_1_before = b7_1;
    bool r7 = stack_allocator_impl::reallocate(p7, buffer7, 1024, 16, b7_1, 128);
    assert(r7 && "Expanding non-last block with space must succeed");
    check_data_preserved(b7_1_before, b7_1, test_data, sizeof(test_data));
    check_alignment_maintained(b7_1, 16, buffer7);
    
    // Test 8: Failed expansion (out of memory)
    alignas(16) char buffer8[256];
    char* p8 = buffer8;
    block b8 = stack_allocator_impl::allocate(p8, buffer8, 256, 16, 128);
    block b8_before = b8;
    bool r8 = stack_allocator_impl::reallocate(p8, buffer8, 256, 16, b8, 1000);
    assert(!r8 && "Reallocation exceeding buffer must fail");
    check_failure_preserves(b8_before, b8, r8);
    
    // Test 9: Multiple reallocations with data preservation
    alignas(16) char buffer9[2048];
    char* p9 = buffer9;
    block b9 = stack_allocator_impl::allocate(p9, buffer9, 2048, 16, 32);
    
    const char data9[] = "persistent_data_across_reallocations";
    std::memcpy(b9.ptr, data9, sizeof(data9));
    
    // Grow
    block b9_before = b9;
    stack_allocator_impl::reallocate(p9, buffer9, 2048, 16, b9, 64);
    check_data_preserved(b9_before, b9, data9, sizeof(data9));
    
    // Grow again
    b9_before = b9;
    stack_allocator_impl::reallocate(p9, buffer9, 2048, 16, b9, 128);
    check_data_preserved(b9_before, b9, data9, sizeof(data9));
    
    // Shrink
    b9_before = b9;
    stack_allocator_impl::reallocate(p9, buffer9, 2048, 16, b9, 48);
    check_data_preserved(b9_before, b9, data9, sizeof(data9));
    
    // Test 10: Reallocation with different alignments
    alignas(32) char buffer10[1024];
    char* p10 = buffer10;
    
    block b10_a = stack_allocator_impl::allocate(p10, buffer10, 1024, 8, 10);
    bool r10_a = stack_allocator_impl::reallocate(p10, buffer10, 1024, 8, b10_a, 20);
    check_alignment_maintained(b10_a, 8, buffer10);
    
    block b10_b = stack_allocator_impl::allocate(p10, buffer10, 1024, 32, 50);
    bool r10_b = stack_allocator_impl::reallocate(p10, buffer10, 1024, 32, b10_b, 100);
    check_alignment_maintained(b10_b, 32, buffer10);
    
    // Test 11: Stress test - many reallocations
    alignas(16) char buffer11[4096];
    char* p11 = buffer11;
    block b11 = stack_allocator_impl::allocate(p11, buffer11, 4096, 16, 16);
    
    const char stress_data[] = "stress_test_marker";
    std::memcpy(b11.ptr, stress_data, sizeof(stress_data));
    
    // Grow progressively
    for (size_t size : {32, 64, 128, 256, 512, 1024}) {
        block b11_before = b11;
        bool result = stack_allocator_impl::reallocate(p11, buffer11, 4096, 16, b11, size);
        assert(result && "Progressive growth must succeed with sufficient space");
        check_data_preserved(b11_before, b11, stress_data, sizeof(stress_data));
        check_alignment_maintained(b11, 16, buffer11);
    }
    
    // Shrink progressively
    for (size_t size : {512, 256, 128, 64, 32, 16}) {
        bool result = stack_allocator_impl::reallocate(p11, buffer11, 4096, 16, b11, size);
        check_shrink_succeeds(result, b11, size, 16);
        check_data_preserved({b11.ptr, size}, b11, stress_data, std::min(sizeof(stress_data), size));
    }
    
    return 0;
}
