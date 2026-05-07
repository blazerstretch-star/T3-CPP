#include "../src/allocator_functions.h"
#include <cassert>

using namespace alb;

int main() {
    // POSITIVE CASES: Successful allocations
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 16, 64);
        assert(b.ptr == buffer && "Normal allocation");
        assert(b.length == 64 && "Correct size");
        assert(p == buffer + 64 && "Pointer advanced");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 16, 10);
        assert(b.length == 16 && "Rounded to alignment");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        assert(b && "Large allocation succeeds");
        assert(b.length % 8 == 0 && "8-byte aligned");
    }
    
    // NEGATIVE CASES: Failed allocations
    {
        char buffer[10];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 10, 16, 100);
        assert(!b && "Insufficient space");
    }
    
    {
        char buffer[1024];
        char* p = buffer + 1000;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 16, 100);
        assert(!b && "No space at position");
    }
    
    // EDGE CASES: Special conditions
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 16, 0);
        assert(!b && "Zero size");
        assert(p == buffer && "Pointer unchanged");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 1, 1);
        assert(b && "Minimum allocation");
        assert(b.length >= 1 && "At least 1 byte");
    }
    
    // BOUNDARY CASES: Limits
    {
        char buffer[128];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 128, 8, 128);
        assert(b && "Exact buffer size");
        assert(p == buffer + 128 && "At end");
    }
    
    {
        char buffer[128];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 128, 8, 64);
        block b2 = stack_allocator_impl::allocate(p, buffer, 128, 8, 64);
        assert(b1 && b2 && "Sequential allocations");
        block b3 = stack_allocator_impl::allocate(p, buffer, 128, 8, 1);
        assert(!b3 && "No space left");
    }
    
    return 0;
}
