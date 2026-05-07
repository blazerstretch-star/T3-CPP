#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <climits>

using namespace MovieRecommendation;

class TestCleanup {
public:
    DLLNode<int>* start;
    DLLNode<int>* last;
    
    TestCleanup() : start(nullptr), last(nullptr) {}
    
    ~TestCleanup() {
        while (start) {
            DLLNode<int>* temp = start;
            start = start->next;
            delete temp;
        }
    }
};

// PROPERTY: Verify DLL integrity
bool verify_dll_integrity(DLLNode<int>* start, DLLNode<int>* last, int expectedCount) {
    if (expectedCount == 0) return start == nullptr && last == nullptr;
    if (start == nullptr || last == nullptr) return false;
    
    // Forward traversal
    int forwardCount = 0;
    DLLNode<int>* current = start;
    while (current) {
        forwardCount++;
        if (current->next == nullptr && current != last) return false;
        if (current->prev && current->prev->next != current) return false;
        current = current->next;
    }
    
    // Backward traversal
    int backwardCount = 0;
    current = last;
    while (current) {
        backwardCount++;
        if (current->prev == nullptr && current != start) return false;
        if (current->next && current->next->prev != current) return false;
        current = current->prev;
    }
    
    return forwardCount == expectedCount && backwardCount == expectedCount;
}

int main() {
    std::cout << "Testing dll_push_front..." << std::endl;
    
    // Test 1: Push to empty list
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_front(test.start, test.last, 42, numElements);
        
        assert(test.start != nullptr && "Start should not be NULL");
        assert(test.last == test.start && "Last should equal start for single element");
        assert(test.start->data == 42);
        assert(test.start->next == nullptr);
        assert(test.start->prev == nullptr);
        assert(numElements == 1);
        assert(verify_dll_integrity(test.start, test.last, 1));
    }
    
    // Test 2: Multiple push_front operations
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 1; i <= 10; i++) {
            dll_push_front(test.start, test.last, i, numElements);
            assert(numElements == i);
            assert(test.start->data == i && "Most recent should be at front");
            assert(verify_dll_integrity(test.start, test.last, i));
        }
        
        // Verify order (should be 10, 9, 8, ..., 1)
        DLLNode<int>* current = test.start;
        for (int i = 10; i >= 1; i--) {
            assert(current->data == i);
            current = current->next;
        }
    }
    
    // Test 3: Extreme values
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_front(test.start, test.last, INT_MAX, numElements);
        dll_push_front(test.start, test.last, INT_MIN, numElements);
        dll_push_front(test.start, test.last, 0, numElements);
        
        assert(numElements == 3);
        assert(test.start->data == 0);
        assert(test.start->next->data == INT_MIN);
        assert(test.last->data == INT_MAX);
        assert(verify_dll_integrity(test.start, test.last, 3));
    }
    
    // Test 4: Large scale
    {
        TestCleanup test;
        int numElements = 0;
        const int SIZE = 1000;
        
        for (int i = 0; i < SIZE; i++) {
            dll_push_front(test.start, test.last, i, numElements);
        }
        
        assert(numElements == SIZE);
        assert(verify_dll_integrity(test.start, test.last, SIZE));
    }
    
    std::cout << "✅ dll_push_front strict test passed (4 test cases)" << std::endl;
    
    // ========================================
    // Test dll_pop_front
    // ========================================
    std::cout << "Testing dll_pop_front..." << std::endl;
    
    // Test 5: Pop from empty list (should not crash)
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_pop_front(test.start, test.last, numElements);
        
        assert(test.start == nullptr);
        assert(test.last == nullptr);
        assert(numElements == 0);
    }
    
    // Test 6: Pop single element
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_front(test.start, test.last, 100, numElements);
        dll_pop_front(test.start, test.last, numElements);
        
        assert(test.start == nullptr);
        assert(test.last == nullptr);
        assert(numElements == 0);
    }
    
    // Test 7: Pop from two-element list
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_front(test.start, test.last, 10, numElements);
        dll_push_front(test.start, test.last, 20, numElements);
        
        dll_pop_front(test.start, test.last, numElements);
        
        assert(test.start == test.last);
        assert(test.start->data == 10);
        assert(test.start->prev == nullptr);
        assert(test.start->next == nullptr);
        assert(numElements == 1);
    }
    
    // Test 8: Multiple pops
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 1; i <= 10; i++) {
            dll_push_back(test.start, test.last, i, numElements);
        }
        
        for (int i = 1; i <= 5; i++) {
            dll_pop_front(test.start, test.last, numElements);
            assert(numElements == 10 - i);
            assert(verify_dll_integrity(test.start, test.last, 10 - i));
        }
        
        assert(test.start->data == 6);
        assert(test.last->data == 10);
    }
    
    // Test 9: Pop all elements
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 0; i < 100; i++) {
            dll_push_back(test.start, test.last, i, numElements);
        }
        
        for (int i = 0; i < 100; i++) {
            dll_pop_front(test.start, test.last, numElements);
        }
        
        assert(test.start == nullptr);
        assert(test.last == nullptr);
        assert(numElements == 0);
    }
    
    // Test 10: Pop after push operations
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_front(test.start, test.last, 1, numElements);
        dll_push_front(test.start, test.last, 2, numElements);
        dll_push_back(test.start, test.last, 3, numElements);
        
        dll_pop_front(test.start, test.last, numElements);
        
        assert(numElements == 2);
        assert(test.start->data == 1);
        assert(test.last->data == 3);
        assert(verify_dll_integrity(test.start, test.last, 2));
    }
    
    std::cout << "✅ dll_pop_front strict test passed (6 test cases)" << std::endl;
    std::cout << "✅ All DLL tests passed (10 total test cases)" << std::endl;
    
    return 0;
}
