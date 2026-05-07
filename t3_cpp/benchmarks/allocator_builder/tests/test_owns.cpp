#include "../src/allocator_functions.h"
#include <cassert>

using namespace alb;

int main() {
    char buffer[1024];
    
    // POSITIVE CASES: Valid ownership
    block b1{buffer + 100, 64};
    assert(stack_allocator_impl::owns(buffer, 1024, b1) == true && "Block in middle");
    
    block b2{buffer, 16};
    assert(stack_allocator_impl::owns(buffer, 1024, b2) == true && "Block at start");
    
    block b3{buffer + 1000, 16};
    assert(stack_allocator_impl::owns(buffer, 1024, b3) == true && "Block near end");
    
    // NEGATIVE CASES: Invalid ownership
    block b4{buffer + 2000, 64};
    assert(stack_allocator_impl::owns(buffer, 1024, b4) == false && "Block outside buffer");
    
    char external[100];
    block b5{external, 50};
    assert(stack_allocator_impl::owns(buffer, 1024, b5) == false && "External block");
    
    // EDGE CASES: Special conditions
    block b6{nullptr, 0};
    assert(stack_allocator_impl::owns(buffer, 1024, b6) == false && "Null block");
    
    block b7{buffer + 1023, 1};
    assert(stack_allocator_impl::owns(buffer, 1024, b7) == true && "Last byte");
    
    // BOUNDARY CASES: Limits
    block b8{buffer + 1024, 0};
    assert(stack_allocator_impl::owns(buffer, 1024, b8) == false && "Exactly at end");
    
    block b9{buffer - 1, 10};
    assert(stack_allocator_impl::owns(buffer, 1024, b9) == false && "Before start");
    
    return 0;
}
