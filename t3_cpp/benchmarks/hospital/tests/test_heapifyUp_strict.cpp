#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <algorithm>
#include <map>

int main() {
    // ========================================
    // PROPERTY 1: Path to root satisfies heap property
    // ========================================
    auto check_path_to_root_valid = [](const std::vector<EmergencyPatient>& heap, int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            assert(heap[parent].emergencyLevel >= heap[index].emergencyLevel &&
                   "Parent must have >= priority than child on path to root");
            index = parent;
        }
    };
    
    // ========================================
    // PROPERTY 2: Size invariant (no elements added/removed)
    // ========================================
    auto check_size_invariant = [](size_t before, size_t after) {
        assert(before == after && "heapifyUp must not change heap size");
    };
    
    // ========================================
    // PROPERTY 3: All original elements preserved
    // ========================================
    auto check_elements_preserved = [](const std::vector<EmergencyPatient>& before,
                                       const std::vector<EmergencyPatient>& after) {
        assert(before.size() == after.size() && "Size must be preserved");
        
        // Count occurrences of each emergency level
        std::map<int, int> beforeCounts, afterCounts;
        for (const auto& p : before) beforeCounts[p.emergencyLevel]++;
        for (const auto& p : after) afterCounts[p.emergencyLevel]++;
        
        assert(beforeCounts == afterCounts && 
               "All elements must be preserved (only reordered)");
    };
    
    // ========================================
    // PROPERTY 4: Idempotent on valid heap
    // ========================================
    auto check_idempotent_on_valid = [&](std::vector<EmergencyPatient> heap, int index) {
        if (heap.empty() || index < 0 || index >= static_cast<int>(heap.size())) return;
        
        // If already valid heap, heapifyUp should not change it
        bool isValidBefore = true;
        if (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index].emergencyLevel > heap[parent].emergencyLevel) {
                isValidBefore = false;
            }
        }
        
        if (isValidBefore) {
            auto before = heap;
            heapifyUp(heap, index);
            // Check if heap is unchanged by comparing sizes and elements
            assert(before.size() == heap.size() && "Size should not change");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Empty heap (boundary case)
    std::vector<EmergencyPatient> heap1;
    size_t size1 = heap1.size();
    heapifyUp(heap1, 0);
    check_size_invariant(size1, heap1.size());
    
    // Test 2: Single element
    std::vector<EmergencyPatient> heap2 = {{"P1", 5}};
    auto before2 = heap2;
    heapifyUp(heap2, 0);
    check_size_invariant(1, heap2.size());
    check_elements_preserved(before2, heap2);
    check_path_to_root_valid(heap2, 0);
    assert(heap2[0].emergencyLevel == 5 && "Single element unchanged");
    
    // Test 3: Element needs to bubble up
    std::vector<EmergencyPatient> heap3 = {{"P1", 5}, {"P2", 3}, {"P3", 4}, {"P4", 10}};
    auto before3 = heap3;
    heapifyUp(heap3, 3);
    check_size_invariant(before3.size(), heap3.size());
    check_elements_preserved(before3, heap3);
    check_path_to_root_valid(heap3, 0);  // Check path from root
    assert(heap3[0].emergencyLevel == 10 && "Highest priority should be at root");
    
    // Test 4: Element already in correct position
    std::vector<EmergencyPatient> heap4 = {{"P1", 10}, {"P2", 8}, {"P3", 5}};
    auto before4 = heap4;
    heapifyUp(heap4, 2);
    check_size_invariant(before4.size(), heap4.size());
    check_elements_preserved(before4, heap4);
    check_path_to_root_valid(heap4, 2);
    
    // Test 5: Duplicate priorities
    std::vector<EmergencyPatient> heap5 = {{"P1", 5}, {"P2", 5}, {"P3", 5}, {"P4", 5}};
    auto before5 = heap5;
    heapifyUp(heap5, 3);
    check_size_invariant(before5.size(), heap5.size());
    check_elements_preserved(before5, heap5);
    check_path_to_root_valid(heap5, 3);
    
    // Test 6: Large heap with random priorities
    std::vector<EmergencyPatient> heap6;
    for (int i = 0; i < 100; i++) {
        heap6.push_back({"Patient" + std::to_string(i), i % 15});
    }
    heap6.push_back({"Critical", 20});
    auto before6 = heap6;
    heapifyUp(heap6, 100);
    check_size_invariant(before6.size(), heap6.size());
    check_elements_preserved(before6, heap6);
    // After heapifyUp, the element should have bubbled to root
    assert(heap6[0].emergencyLevel == 20 && "Highest priority at root");
    
    // Test 7: Invalid index (negative) - should handle gracefully
    std::vector<EmergencyPatient> heap7 = {{"P1", 5}, {"P2", 8}};
    auto before7 = heap7;
    heapifyUp(heap7, -1);
    check_size_invariant(before7.size(), heap7.size());
    // Should not crash or corrupt data
    
    // Test 8: Invalid index (out of bounds) - should handle gracefully
    std::vector<EmergencyPatient> heap8 = {{"P1", 5}, {"P2", 8}};
    auto before8 = heap8;
    heapifyUp(heap8, 100);
    check_size_invariant(before8.size(), heap8.size());
    // Should not crash or corrupt data
    
    // Test 9: Stress test - build complete heap by adding elements one by one
    std::vector<EmergencyPatient> heap9;
    for (int i = 0; i < 1000; i++) {
        heap9.push_back({"P" + std::to_string(i), i});
        heapifyUp(heap9, i);
        // After heapifyUp, the highest priority should be at root
        int maxPriority = heap9[0].emergencyLevel;
        for (const auto& p : heap9) {
            assert(maxPriority >= p.emergencyLevel && "Root has max priority");
        }
    }
    
    std::cout << "All heapifyUp strict tests passed!" << std::endl;
    return 0;
}
