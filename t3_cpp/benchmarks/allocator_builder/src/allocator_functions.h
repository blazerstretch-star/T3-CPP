#pragma once
#include <cstddef>
#include <cstdint>

namespace alb {

// Block structure for memory allocation
struct block {
    void* ptr = nullptr;
    size_t length = 0;
    
    explicit operator bool() const { return ptr != nullptr; }
    void reset() { ptr = nullptr; length = 0; }
};

// Stack allocator state
template<size_t MaxSize, size_t Alignment = 16>
struct stack_allocator_state {
    alignas(Alignment) char data[MaxSize];
    char* p = data;
    static constexpr size_t max_size = MaxSize;
    static constexpr size_t alignment = Alignment;
};

// Function declarations
namespace stack_allocator_impl {
    bool owns(const void* data, size_t max_size, const block& b);
    block allocate(char*& p, const char* data, size_t max_size, size_t alignment, size_t n);
    void deallocate(char*& p, const char* data, size_t max_size, block& b);
    bool reallocate(char*& p, const char* data, size_t max_size, size_t alignment, block& b, size_t n);
}

namespace segregator_impl {
    template<typename SmallAlloc, typename LargeAlloc>
    void deallocate(SmallAlloc& small, LargeAlloc& large, size_t threshold, block& b);
    
    template<typename SmallAlloc, typename LargeAlloc>
    block allocate(SmallAlloc& small, LargeAlloc& large, size_t threshold, size_t n);
}

namespace bucketizer_impl {
    template<typename Allocator, size_t NumBuckets>
    block allocate(Allocator* buckets, size_t num_buckets, size_t n);
    
    template<typename Allocator>
    bool reallocate(Allocator* buckets, size_t num_buckets, size_t min_size, size_t step_size, block& b, size_t n);
}

namespace freelist_impl {
    template<typename Stack, typename Allocator>
    block allocate(Stack& root, Allocator& allocator, size_t lower_bound, size_t upper_bound, 
                   size_t batch_size, bool supports_truncated);
}

namespace affix_impl {
    template<typename Allocator>
    block allocate(Allocator& allocator, size_t n, size_t prefix_size, size_t sufix_size, size_t alignment);
}

} // namespace alb
