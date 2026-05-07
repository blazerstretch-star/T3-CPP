#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <climits>
#include <vector>
#include <set>
#include <algorithm>

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

// PROPERTY 1: Forward-backward consistency
bool verify_forward_backward_consistency(DLLNode<int>* start, DLLNode<int>* last) {
    if (!start && !last) return true;
    if (!start || !last) return false;
    
    // Forward traversal
    std::vector<DLLNode<int>*> forward;
    DLLNode<int>* curr = start;
    while (curr) {
        forward.push_back(curr);
        curr = curr->next;
    }
    
    // Backward traversal
    std::vector<DLLNode<int>*> backward;
    curr = last;
    while (curr) {
        backward.push_back(curr);
        curr = curr->prev;
    }
    
    // Should be reverse of each other
    std::reverse(backward.begin(), backward.end());
    return forward == backward;
}

// PROPERTY 2: Pointer consistency
bool verify_pointer_consistency(DLLNode<int>* start, DLLNode<int>* last) {
    if (!start) return true;
    
    DLLNode<int>* curr = start;
    while (curr) {
        // If has next, next->prev should point back
        if (curr->next && curr->next->prev != curr) return false;
        
        // If has prev, prev->next should point forward
        if (curr->prev && curr->prev->next != curr) return false;
        
        // First node has no prev
        if (curr == start && curr->prev != nullptr) return false;
        
        // Last node has no next
        if (curr == last && curr->next != nullptr) return false;
        
        curr = curr->next;
    }
    
    return true;
}

// PROPERTY 3: Count consistency
bool verify_count(DLLNode<int>* start, int expectedCount) {
    int count = 0;
    DLLNode<int>* curr = start;
    while (curr) {
        count++;
        curr = curr->next;
    }
    return count == expectedCount;
}

// PROPERTY 4: No cycles
bool verify_no_cycles(DLLNode<int>* start) {
    std::set<DLLNode<int>*> visited;
    DLLNode<int>* curr = start;
    
    while (curr) {
        if (visited.count(curr)) return false; // Cycle detected
        visited.insert(curr);
        curr = curr->next;
    }
    
    return true;
}

// PROPERTY 5: Data order preserved
bool verify_data_order(DLLNode<int>* start, const std::vector<int>& expected) {
    std::vector<int> actual;
    DLLNode<int>* curr = start;
    while (curr) {
        actual.push_back(curr->data);
        curr = curr->next;
    }
    return actual == expected;
}

// Combined verification
bool verify_dll_integrity(DLLNode<int>* start, DLLNode<int>* last, int expectedCount) {
    if (expectedCount == 0) return start == nullptr && last == nullptr;
    if (!start || !last) return false;
    
    return verify_forward_backward_consistency(start, last) &&
           verify_pointer_consistency(start, last) &&
           verify_count(start, expectedCount) &&
           verify_no_cycles(start);
}

int main() {
    // Test 1: Empty list - first insertion
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_back(test.start, test.last, 42, numElements);
        
        assert(test.start != nullptr && "Start should not be NULL");
        assert(test.last == test.start && "Last should equal start for single element");
        assert(test.start->data == 42);
        assert(test.start->next == nullptr);
        assert(test.start->prev == nullptr);
        assert(numElements == 1);
        assert(verify_dll_integrity(test.start, test.last, 1));
    }
    
    // Test 2: Two insertions
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_back(test.start, test.last, 10, numElements);
        dll_push_back(test.start, test.last, 20, numElements);
        
        assert(numElements == 2);
        assert(test.start->data == 10);
        assert(test.last->data == 20);
        assert(test.start->next == test.last);
        assert(test.last->prev == test.start);
        assert(verify_dll_integrity(test.start, test.last, 2));
    }
    
    // Test 3: Multiple insertions (100 elements)
    {
        TestCleanup test;
        int numElements = 0;
        std::vector<int> expected;
        
        for (int i = 1; i <= 100; i++) {
            dll_push_back(test.start, test.last, i, numElements);
            expected.push_back(i);
            assert(numElements == i);
            assert(test.last->data == i && "Last element should be most recent");
            assert(verify_dll_integrity(test.start, test.last, i));
        }
        
        assert(verify_data_order(test.start, expected));
    }
    
    // Test 4: Extreme values
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_back(test.start, test.last, INT_MAX, numElements);
        dll_push_back(test.start, test.last, INT_MIN, numElements);
        dll_push_back(test.start, test.last, 0, numElements);
        dll_push_back(test.start, test.last, -1, numElements);
        dll_push_back(test.start, test.last, 1, numElements);
        
        assert(numElements == 5);
        assert(test.start->data == INT_MAX);
        assert(test.last->data == 1);
        assert(verify_dll_integrity(test.start, test.last, 5));
        
        std::vector<int> expected = {INT_MAX, INT_MIN, 0, -1, 1};
        assert(verify_data_order(test.start, expected));
    }
    
    // Test 5: Pointer consistency after each insertion
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_back(test.start, test.last, 10, numElements);
        DLLNode<int>* first = test.start;
        
        dll_push_back(test.start, test.last, 20, numElements);
        assert(test.start == first && "Start shouldn't change");
        assert(first->next == test.last);
        assert(test.last->prev == first);
        assert(verify_pointer_consistency(test.start, test.last));
        
        dll_push_back(test.start, test.last, 30, numElements);
        assert(test.start == first && "Start still shouldn't change");
        assert(test.last->data == 30);
        assert(test.last->prev->data == 20);
        assert(verify_pointer_consistency(test.start, test.last));
    }
    
    // Test 6: Large scale insertion (10,000 elements)
    {
        TestCleanup test;
        int numElements = 0;
        const int LARGE_SIZE = 10000;
        
        for (int i = 0; i < LARGE_SIZE; i++) {
            dll_push_back(test.start, test.last, i * 2, numElements);
            
            // Verify every 1000 insertions
            if (i % 1000 == 999) {
                assert(verify_dll_integrity(test.start, test.last, i + 1));
            }
        }
        
        assert(numElements == LARGE_SIZE);
        assert(verify_dll_integrity(test.start, test.last, LARGE_SIZE));
        
        // Verify all values
        DLLNode<int>* current = test.start;
        for (int i = 0; i < LARGE_SIZE; i++) {
            assert(current->data == i * 2);
            current = current->next;
        }
    }
    
    // Test 7: Alternating positive/negative
    {
        TestCleanup test;
        int numElements = 0;
        std::vector<int> expected;
        
        for (int i = 0; i < 50; i++) {
            int value = (i % 2 == 0) ? i : -i;
            dll_push_back(test.start, test.last, value, numElements);
            expected.push_back(value);
        }
        
        assert(numElements == 50);
        assert(verify_dll_integrity(test.start, test.last, 50));
        assert(verify_data_order(test.start, expected));
    }
    
    // Test 8: All identical values
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 0; i < 100; i++) {
            dll_push_back(test.start, test.last, 777, numElements);
        }
        
        assert(numElements == 100);
        assert(verify_dll_integrity(test.start, test.last, 100));
        
        DLLNode<int>* curr = test.start;
        for (int i = 0; i < 100; i++) {
            assert(curr->data == 777);
            curr = curr->next;
        }
    }
    
    // Test 9: Verify no memory corruption
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 0; i < 1000; i++) {
            dll_push_back(test.start, test.last, i, numElements);
        }
        
        // Traverse and verify all pointers are valid
        DLLNode<int>* curr = test.start;
        int count = 0;
        while (curr) {
            assert(curr->data == count);
            count++;
            curr = curr->next;
        }
        
        assert(count == 1000);
        assert(verify_no_cycles(test.start));
    }
    
    // Test 10: Backward traversal verification
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int i = 1; i <= 50; i++) {
            dll_push_back(test.start, test.last, i, numElements);
        }
        
        // Traverse backward
        DLLNode<int>* curr = test.last;
        for (int i = 50; i >= 1; i--) {
            assert(curr != nullptr);
            assert(curr->data == i);
            curr = curr->prev;
        }
        
        assert(curr == nullptr && "Should reach start");
    }
    
    // Test 11: Mixed with push_front operations
    {
        TestCleanup test;
        int numElements = 0;
        
        dll_push_back(test.start, test.last, 2, numElements);
        dll_push_front(test.start, test.last, 1, numElements);
        dll_push_back(test.start, test.last, 3, numElements);
        dll_push_front(test.start, test.last, 0, numElements);
        dll_push_back(test.start, test.last, 4, numElements);
        
        assert(numElements == 5);
        assert(verify_dll_integrity(test.start, test.last, 5));
        
        std::vector<int> expected = {0, 1, 2, 3, 4};
        assert(verify_data_order(test.start, expected));
    }
    
    // Test 12: Stress test - rapid insertions
    {
        TestCleanup test;
        int numElements = 0;
        
        for (int batch = 0; batch < 10; batch++) {
            for (int i = 0; i < 100; i++) {
                dll_push_back(test.start, test.last, batch * 100 + i, numElements);
            }
            assert(verify_dll_integrity(test.start, test.last, (batch + 1) * 100));
        }
        
        assert(numElements == 1000);
    }
    
    std::cout << "✅ dll_push_back ultra-strict test passed (12 test cases)" << std::endl;
    std::cout << "   - Empty list insertion verified" << std::endl;
    std::cout << "   - Multiple insertions (100 elements) verified" << std::endl;
    std::cout << "   - Extreme values verified" << std::endl;
    std::cout << "   - Pointer consistency verified" << std::endl;
    std::cout << "   - Large scale (10,000 elements) verified" << std::endl;
    std::cout << "   - Forward-backward consistency verified" << std::endl;
    std::cout << "   - No cycles verified" << std::endl;
    std::cout << "   - Data order preserved" << std::endl;
    std::cout << "   - Backward traversal verified" << std::endl;
    std::cout << "   - Mixed operations verified" << std::endl;
    return 0;
}
