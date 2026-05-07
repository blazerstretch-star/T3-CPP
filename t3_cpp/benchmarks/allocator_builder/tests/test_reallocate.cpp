#include "../src/allocator_functions.h"
#include <cassert>
#include <cstring>

using namespace alb;

int main() {
    // POSITIVE CASES: Successful reallocations
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 100);
        assert(result && "Same size succeeds");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 200);
        assert(result && "Expand last block");
        assert(b.length >= 200 && "New size");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 50);
        assert(result && "Shrink succeeds");
    }
    
    // NEGATIVE CASES: Failed reallocations
    {
        char buffer[100];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 100, 8, 50);
        bool result = stack_allocator_impl::reallocate(p, buffer, 100, 8, b, 200);
        assert(!result && "Insufficient space");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        block b2 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 800);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b1, 200);
        assert(!result && "Non-last block expand fails");
    }
    
    // EDGE CASES: Special conditions
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 0);
        assert(result && "Zero size deallocates");
        assert(!b && "Block reset");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b{nullptr, 0};
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 100);
        assert(result && "Null block allocates");
        assert(b && "Block created");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        block b2 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        std::memset(b1.ptr, 0xAA, b1.length);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b1, 200);
        assert(result && "Non-last reallocates with copy");
    }
    
    // BOUNDARY CASES: Limits
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 512);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 1024);
        assert(result && "Expand to full buffer");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 1024);
        bool result = stack_allocator_impl::reallocate(p, buffer, 1024, 8, b, 1);
        assert(result && "Shrink to minimum");
    }
    
    return 0;
}
