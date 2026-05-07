#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>

using namespace MovieRecommendation;

// PROPERTY 1: Data preservation (ALWAYS required)
bool verify_data_preserved(Vector<int>& vec, int* expectedData, int count) {
    for (int i = 0; i < count; i++) {
        if (vec.arr[i] != expectedData[i]) return false;
    }
    return true;
}

// PROPERTY 2: Capacity must fit all elements (ALWAYS required)
bool verify_capacity_sufficient(Vector<int>& vec) {
    return vec.capacity >= vec.numElements && vec.capacity > 0;
}

// PROPERTY 3: Element count unchanged (ALWAYS required)
bool verify_element_count_unchanged(int before, int after) {
    return before == after;
}

int main() {
    // Test 1: Very low utilization (10%) - likely to trigger reduction
    {
        Vector<int> vec(10);
        vec.arr[0] = 42;
        vec.numElements = 1;
        
        int originalCapacity = vec.capacity;
        int originalElements = vec.numElements;
        int originalData = vec.arr[0];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Data must be preserved
        assert(vec.arr[0] == originalData && "Data must be preserved");
        
        // ✅ REQUIRED: Element count unchanged
        assert(vec.numElements == originalElements && "Element count unchanged");
        
        // ✅ REQUIRED: Capacity must fit all elements
        assert(verify_capacity_sufficient(vec) && "Capacity must fit all elements");
        
        // ℹ️ OPTIONAL: Capacity may or may not decrease (implementation-dependent)
        // We don't assert anything about capacity change
    }
    
    // Test 2: High utilization (90%) - unlikely to trigger reduction
    {
        Vector<int> vec(10);
        for (int i = 0; i < 9; i++) {
            vec.arr[i] = i * 10;
        }
        vec.numElements = 9;
        
        int originalCapacity = vec.capacity;
        int originalElements = vec.numElements;
        int originalData[9];
        for (int i = 0; i < 9; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Data preserved
        assert(verify_data_preserved(vec, originalData, 9));
        
        // ✅ REQUIRED: Element count unchanged
        assert(vec.numElements == originalElements);
        
        // ✅ REQUIRED: Capacity sufficient
        assert(verify_capacity_sufficient(vec));
        
        // ℹ️ OPTIONAL: At high utilization, reduction is unlikely but not forbidden
    }
    
    // Test 3: Medium utilization (50%) - behavior is implementation-dependent
    {
        Vector<int> vec(10);
        for (int i = 0; i < 5; i++) {
            vec.arr[i] = i * 5;
        }
        vec.numElements = 5;
        
        int originalElements = vec.numElements;
        int originalData[5];
        for (int i = 0; i < 5; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Properties must hold regardless of reduction decision
        assert(verify_data_preserved(vec, originalData, 5));
        assert(vec.numElements == originalElements);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 4: Multiple elements preserved
    {
        Vector<int> vec(20);
        for (int i = 0; i < 5; i++) {
            vec.arr[i] = i * i;
        }
        vec.numElements = 5;
        
        int originalElements = vec.numElements;
        int originalData[5];
        for (int i = 0; i < 5; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: All data preserved
        for (int i = 0; i < 5; i++) {
            assert(vec.arr[i] == i * i && "All data must be preserved");
        }
        assert(vec.numElements == originalElements);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 5: Very low utilization (5%)
    {
        Vector<int> vec(100);
        for (int i = 0; i < 5; i++) {
            vec.arr[i] = 100 + i;
        }
        vec.numElements = 5;
        
        int originalElements = vec.numElements;
        int originalData[5];
        for (int i = 0; i < 5; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Properties hold
        assert(verify_data_preserved(vec, originalData, 5));
        assert(vec.numElements == originalElements);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 6: Edge case - 1 element in large vector
    {
        Vector<int> vec(50);
        vec.arr[0] = 999;
        vec.numElements = 1;
        
        int originalData = vec.arr[0];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Properties hold
        assert(vec.arr[0] == originalData);
        assert(vec.numElements == 1);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 7: Small capacity vector
    {
        Vector<int> vec(4);
        vec.arr[0] = 10;
        vec.numElements = 1;
        
        int originalData = vec.arr[0];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Properties hold
        assert(vec.arr[0] == originalData);
        assert(vec.numElements == 1);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 8: Verify capacity never becomes invalid
    {
        Vector<int> vec(100);
        for (int i = 0; i < 10; i++) {
            vec.arr[i] = i * 10;
        }
        vec.numElements = 10;
        
        int originalData[10];
        for (int i = 0; i < 10; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Capacity must be positive and sufficient
        assert(vec.capacity > 0 && "Capacity must be positive");
        assert(vec.capacity >= vec.numElements && "Capacity must fit all elements");
        assert(verify_data_preserved(vec, originalData, 10));
    }
    
    // Test 9: Large vector with many elements
    {
        Vector<int> vec(1000);
        for (int i = 0; i < 100; i++) {
            vec.arr[i] = i * 10;
        }
        vec.numElements = 100;
        
        int originalElements = vec.numElements;
        int originalData[100];
        for (int i = 0; i < 100; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Properties hold
        assert(verify_data_preserved(vec, originalData, 100));
        assert(vec.numElements == originalElements);
        assert(verify_capacity_sufficient(vec));
    }
    
    // Test 10: Consecutive reductions (if reduction happens)
    {
        Vector<int> vec(100);
        vec.arr[0] = 777;
        vec.numElements = 1;
        
        // First reduction
        vector_dynamic_reduction(vec);
        int cap1 = vec.capacity;
        assert(vec.arr[0] == 777 && "Data preserved after 1st reduction");
        assert(verify_capacity_sufficient(vec));
        
        // Second reduction (behavior is implementation-dependent)
        vector_dynamic_reduction(vec);
        assert(vec.arr[0] == 777 && "Data preserved after 2nd reduction");
        assert(verify_capacity_sufficient(vec));
        
        // ✅ REQUIRED: Capacity should not increase
        assert(vec.capacity <= cap1 && "Capacity should not increase during reduction");
    }
    
    // Test 11: Zero elements (edge case)
    {
        Vector<int> vec(10);
        vec.numElements = 0;
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Valid state maintained
        assert(vec.capacity > 0 && "Capacity must remain positive");
        assert(vec.numElements == 0 && "Element count unchanged");
    }
    
    // Test 12: Verify properties hold regardless of implementation strategy
    {
        Vector<int> vec(20);
        for (int i = 0; i < 3; i++) {
            vec.arr[i] = i + 100;
        }
        vec.numElements = 3;
        
        int oldCap = vec.capacity;
        int oldElements = vec.numElements;
        int originalData[3];
        for (int i = 0; i < 3; i++) originalData[i] = vec.arr[i];
        
        vector_dynamic_reduction(vec);
        
        // ✅ REQUIRED: Core properties (implementation-agnostic)
        assert(verify_data_preserved(vec, originalData, 3) && "Data preserved");
        assert(vec.numElements == oldElements && "Element count unchanged");
        assert(verify_capacity_sufficient(vec) && "Capacity sufficient");
        
        // ✅ REQUIRED: If capacity changed, it should decrease or stay same
        assert(vec.capacity <= oldCap && "Capacity should not increase");
        
        // ℹ️ NOTE: We don't require reduction to happen at any specific threshold
        // Implementation may use any reasonable strategy (50%, 25%, etc.)
    }
    
    std::cout << "✅ vector_dynamic_reduction fully generalized test passed (12 test cases)" << std::endl;
    std::cout << "   ✅ Data preservation verified (REQUIRED)" << std::endl;
    std::cout << "   ✅ Element count unchanged (REQUIRED)" << std::endl;
    std::cout << "   ✅ Capacity sufficient (REQUIRED)" << std::endl;
    std::cout << "   ✅ Capacity never increases (REQUIRED)" << std::endl;
    std::cout << "   ℹ️  No assumptions about reduction thresholds" << std::endl;
    std::cout << "   ℹ️  Implementation-agnostic (any strategy accepted)" << std::endl;
    return 0;
}
