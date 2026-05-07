#include "../src/allocator_functions.h"
#include <cassert>

using namespace alb;

int main() {
    // POSITIVE CASES: Successful deallocations
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        assert(p == buffer && "Last block reclaimed");
        assert(!b && "Block reset");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 50);
        block b2 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 50);
        stack_allocator_impl::deallocate(p, buffer, 1024, b2);
        assert(p == static_cast<char*>(b1.ptr) + b1.length && "Last block reclaimed");
    }
    
    // NEGATIVE CASES: No space reclamation
    {
        char buffer[1024];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        block b2 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        char* p_before = p;
        stack_allocator_impl::deallocate(p, buffer, 1024, b1);
        assert(p == p_before && "Non-last block no reclaim");
        assert(!b1 && "Block still reset");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        char external[100];
        block b{external, 50};
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        assert(p == buffer && "External block ignored");
    }
    
    // EDGE CASES: Special conditions
    {
        char buffer[1024];
        char* p = buffer;
        block b{nullptr, 0};
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        assert(p == buffer && "Empty block no-op");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        assert(p == buffer && "Double deallocation safe");
    }
    
    // BOUNDARY CASES: Multiple operations
    {
        char buffer[1024];
        char* p = buffer;
        block b1 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        block b2 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        block b3 = stack_allocator_impl::allocate(p, buffer, 1024, 8, 100);
        
        stack_allocator_impl::deallocate(p, buffer, 1024, b3);
        stack_allocator_impl::deallocate(p, buffer, 1024, b2);
        stack_allocator_impl::deallocate(p, buffer, 1024, b1);
        assert(p == buffer && "LIFO reclaims all");
    }
    
    {
        char buffer[1024];
        char* p = buffer;
        block b = stack_allocator_impl::allocate(p, buffer, 1024, 8, 1024);
        stack_allocator_impl::deallocate(p, buffer, 1024, b);
        assert(p == buffer && "Full buffer deallocation");
    }
    
    return 0;
}
