#include "../src/allocator_functions.h"
#include <cassert>
#include <cstring>
#include <vector>

using namespace alb;

// Helper to check alignment
inline bool is_aligned(void* ptr, size_t alignment) {
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

// Helper to round up to alignment
inline size_t round_to_alignment(size_t alignment, size_t n) {
    return (n + alignment - 1) & ~(alignment - 1);
}

int main() {
    // ========================================
    // PROPERTY 1: Zero-size allocation returns empty block
    // ========================================
    auto check_zero_size_empty = [](char*& p, const char* data, size_t max_size, size_t alignment) {
        char* p_before = p;
        block b = stack_allocator_impl::allocate(p, data, max_size, alignment, 0);
        assert(!b && "Zero-size allocation must return empty block");
        assert(p == p_before && "Zero-size allocation must not move stack pointer");
    };
    
    // ========================================
    // PROPERTY 2: Successful allocation returns valid block
    // ========================================
    auto check_valid_allocation = [](const char* data, size_t max_size, const block& b, size_t requested) {
        if (b) {
            assert(b.ptr != nullptr && "Valid block must have non-null pointer");
            assert(b.length > 0 && "Valid block must have positive length");
            assert(b.length >= requested && "Allocated length must be >= requested size");
            
            // Block must be within buffer bounds
            const char* block_start = static_cast<const char*>(b.ptr);
            const char* block_end = block_start + b.length;
            assert(block_start >= data && "Block start must be within buffer");
            assert(block_end <= data + max_size && "Block end must be within buffer");
        }
    };
    
    // ========================================
    // PROPERTY 3: Allocation respects alignment
    // ========================================
    auto check_alignment_respected = [](const block& b, size_t alignment, size_t requested) {
        if (b) {
            assert(is_aligned(b.ptr, alignment) && 
                   "Allocated block pointer must be aligned");
            assert(b.length % alignment == 0 && 
                   "Allocated block length must be multiple of alignment");
            assert(b.length == round_to_alignment(alignment, requested) && 
                   "Block length must be requested size rounded to alignment");
        }
    };
    
    // ========================================
    // PROPERTY 4: Stack pointer advances correctly
    // ========================================
    auto check_pointer_advance = [](char* p_before, char* p_after, const block& b) {
        if (b) {
            assert(p_after == static_cast<char*>(b.ptr) + b.length && 
                   "Stack pointer must advance by block length");
            assert(p_after > p_before && 
                   "Stack pointer must move forward on successful allocation");
        } else {
            assert(p_after == p_before && 
                   "Stack pointer must not move on failed allocation");
        }
    };
    
    // ========================================
    // PROPERTY 5: Out-of-memory returns empty block
    // ========================================
    auto check_oom_empty = [](char*& p, const char* data, size_t max_size, size_t alignment, size_t n) {
        size_t available = (data + max_size) - p;
        size_t needed = round_to_alignment(alignment, n);
        
        if (needed > available) {
            char* p_before = p;
            block b = stack_allocator_impl::allocate(p, data, max_size, alignment, n);
            assert(!b && "Allocation exceeding available space must return empty block");
            assert(p == p_before && "Failed allocation must not move stack pointer");
        }
    };
    
    // ========================================
    // PROPERTY 6: Sequential allocations don't overlap
    // ========================================
    auto check_no_overlap = [](const block& b1, const block& b2) {
        if (b1 && b2) {
            const char* b1_start = static_cast<const char*>(b1.ptr);
            const char* b1_end = b1_start + b1.length;
            const char* b2_start = static_cast<const char*>(b2.ptr);
            const char* b2_end = b2_start + b2.length;
            
            bool no_overlap = (b1_end <= b2_start) || (b2_end <= b1_start);
            assert(no_overlap && "Sequential allocations must not overlap");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Zero-size allocation
    char buffer1[1024];
    char* p1 = buffer1;
    check_zero_size_empty(p1, buffer1, 1024, 16);
    check_zero_size_empty(p1, buffer1, 1024, 8);
    check_zero_size_empty(p1, buffer1, 1024, 32);
    
    // Test 2: Simple allocation with alignment 16
    char buffer2[1024];
    char* p2 = buffer2;
    char* p2_before = p2;
    block b2 = stack_allocator_impl::allocate(p2, buffer2, 1024, 16, 64);
    check_valid_allocation(buffer2, 1024, b2, 64);
    check_alignment_respected(b2, 16, 64);
    check_pointer_advance(p2_before, p2, b2);
    assert(b2 && "Allocation with sufficient space must succeed");
    
    // Test 3: Allocation requiring alignment padding
    char buffer3[1024];
    char* p3 = buffer3;
    block b3 = stack_allocator_impl::allocate(p3, buffer3, 1024, 16, 10);
    check_valid_allocation(buffer3, 1024, b3, 10);
    check_alignment_respected(b3, 16, 10);
    assert(b3 && b3.length == 16 && "10 bytes with alignment 16 must allocate 16 bytes");
    
    // Test 4: Allocation with alignment 8
    char buffer4[1024];
    char* p4 = buffer4;
    block b4 = stack_allocator_impl::allocate(p4, buffer4, 1024, 8, 13);
    check_valid_allocation(buffer4, 1024, b4, 13);
    check_alignment_respected(b4, 8, 13);
    assert(b4 && b4.length == 16 && "13 bytes with alignment 8 must allocate 16 bytes");
    
    // Test 5: Allocation with alignment 32
    char buffer5[1024];
    char* p5 = buffer5;
    block b5 = stack_allocator_impl::allocate(p5, buffer5, 1024, 32, 50);
    check_valid_allocation(buffer5, 1024, b5, 50);
    check_alignment_respected(b5, 32, 50);
    assert(b5 && b5.length == 64 && "50 bytes with alignment 32 must allocate 64 bytes");
    
    // Test 6: Out of memory - exact boundary
    char buffer6[128];
    char* p6 = buffer6 + 120;
    check_oom_empty(p6, buffer6, 128, 16, 16);
    
    // Test 7: Out of memory - far exceeds
    char buffer7[128];
    char* p7 = buffer7 + 100;
    check_oom_empty(p7, buffer7, 128, 16, 1000);
    
    // Test 8: Sequential allocations don't overlap
    char buffer8[1024];
    char* p8 = buffer8;
    block b8_1 = stack_allocator_impl::allocate(p8, buffer8, 1024, 16, 32);
    block b8_2 = stack_allocator_impl::allocate(p8, buffer8, 1024, 16, 48);
    block b8_3 = stack_allocator_impl::allocate(p8, buffer8, 1024, 16, 64);
    check_no_overlap(b8_1, b8_2);
    check_no_overlap(b8_2, b8_3);
    check_no_overlap(b8_1, b8_3);
    
    // Test 9: Fill entire buffer
    char buffer9[256];
    char* p9 = buffer9;
    std::vector<block> blocks;
    while (true) {
        block b = stack_allocator_impl::allocate(p9, buffer9, 256, 16, 16);
        if (!b) break;
        blocks.push_back(b);
        check_valid_allocation(buffer9, 256, b, 16);
        check_alignment_respected(b, 16, 16);
    }
    assert(blocks.size() == 16 && "256-byte buffer should fit exactly 16 blocks of 16 bytes");
    
    // Verify all blocks are non-overlapping
    for (size_t i = 0; i < blocks.size(); ++i) {
        for (size_t j = i + 1; j < blocks.size(); ++j) {
            check_no_overlap(blocks[i], blocks[j]);
        }
    }
    
    // Test 10: Single byte allocation with alignment 1
    char buffer10[1024];
    char* p10 = buffer10;
    block b10 = stack_allocator_impl::allocate(p10, buffer10, 1024, 1, 1);
    check_valid_allocation(buffer10, 1024, b10, 1);
    assert(b10 && b10.length == 1 && "Single byte with alignment 1 must allocate 1 byte");
    
    // Test 11: Large single allocation
    char buffer11[2048];
    char* p11 = buffer11;
    block b11 = stack_allocator_impl::allocate(p11, buffer11, 2048, 16, 2000);
    check_valid_allocation(buffer11, 2048, b11, 2000);
    check_alignment_respected(b11, 16, 2000);
    assert(b11 && "Large allocation within buffer must succeed");
    
    return 0;
}
