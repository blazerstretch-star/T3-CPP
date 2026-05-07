#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <map>

int main() {
    // ========================================
    // PROPERTY 1: Max-heap property maintained
    // ========================================
    auto check_max_heap_property = [](const std::vector<EmergencyPatient>& heap) {
        for (size_t i = 0; i < heap.size(); ++i) {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            if (left < heap.size()) {
                assert(heap[i].emergencyLevel >= heap[left].emergencyLevel && 
                       "Parent must have >= priority than left child");
            }
            if (right < heap.size()) {
                assert(heap[i].emergencyLevel >= heap[right].emergencyLevel && 
                       "Parent must have >= priority than right child");
            }
        }
    };
    
    // ========================================
    // PROPERTY 2: Size invariant
    // ========================================
    auto check_size_invariant = [](size_t before, size_t after) {
        assert(before == after && "heapifyDown must not change heap size");
    };
    
    // ========================================
    // PROPERTY 3: All elements preserved
    // ========================================
    auto check_elements_preserved = [](const std::vector<EmergencyPatient>& before,
                                       const std::vector<EmergencyPatient>& after) {
        assert(before.size() == after.size() && "Size must be preserved");
        
        std::map<int, int> beforeCounts, afterCounts;
        for (const auto& p : before) beforeCounts[p.emergencyLevel]++;
        for (const auto& p : after) afterCounts[p.emergencyLevel]++;
        
        assert(beforeCounts == afterCounts && 
               "All elements must be preserved (only reordered)");
    };
    
    // ========================================
    // PROPERTY 4: Subtree heap property
    // ========================================
    auto check_subtree_valid = [](const std::vector<EmergencyPatient>& heap, int index) {
        if (index < 0 || index >= static_cast<int>(heap.size())) return;
        
        std::function<void(int)> validate = [&](int i) {
            if (i >= static_cast<int>(heap.size())) return;
            
            int left = 2 * i + 1;
            int right = 2 * i + 2;
            
            if (left < static_cast<int>(heap.size())) {
                assert(heap[i].emergencyLevel >= heap[left].emergencyLevel &&
                       "Subtree must maintain heap property");
                validate(left);
            }
            if (right < static_cast<int>(heap.size())) {
                assert(heap[i].emergencyLevel >= heap[right].emergencyLevel &&
                       "Subtree must maintain heap property");
                validate(right);
            }
        };
        
        validate(index);
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Empty heap
    std::vector<EmergencyPatient> heap1;
    heapifyDown(heap1, 0);
    check_size_invariant(0, heap1.size());
    check_max_heap_property(heap1);
    
    // Test 2: Single element
    std::vector<EmergencyPatient> heap2 = {{"P1", 5}};
    auto before2 = heap2;
    heapifyDown(heap2, 0);
    check_size_invariant(1, heap2.size());
    check_elements_preserved(before2, heap2);
    check_max_heap_property(heap2);
    assert(heap2[0].emergencyLevel == 5 && "Single element unchanged");
    
    // Test 3: Root needs to sink down
    std::vector<EmergencyPatient> heap3 = {{"P1", 3}, {"P2", 8}, {"P3", 5}};
    auto before3 = heap3;
    heapifyDown(heap3, 0);
    check_size_invariant(before3.size(), heap3.size());
    check_elements_preserved(before3, heap3);
    check_max_heap_property(heap3);
    assert(heap3[0].emergencyLevel == 8 && "Max element should be at root");
    
    // Test 4: Already valid heap
    std::vector<EmergencyPatient> heap4 = {{"P1", 10}, {"P2", 8}, {"P3", 5}};
    auto before4 = heap4;
    heapifyDown(heap4, 0);
    check_size_invariant(before4.size(), heap4.size());
    check_elements_preserved(before4, heap4);
    check_max_heap_property(heap4);
    
    // Test 5: Heapify from middle
    std::vector<EmergencyPatient> heap5 = {{"P1", 10}, {"P2", 3}, {"P3", 8}, {"P4", 7}, {"P5", 6}};
    auto before5 = heap5;
    heapifyDown(heap5, 1);
    check_size_invariant(before5.size(), heap5.size());
    check_elements_preserved(before5, heap5);
    check_subtree_valid(heap5, 1);
    
    // Test 6: Duplicate priorities
    std::vector<EmergencyPatient> heap6 = {{"P1", 5}, {"P2", 10}, {"P3", 10}};
    auto before6 = heap6;
    heapifyDown(heap6, 0);
    check_size_invariant(before6.size(), heap6.size());
    check_elements_preserved(before6, heap6);
    check_max_heap_property(heap6);
    assert(heap6[0].emergencyLevel == 10 && "Max priority at root");
    
    // Test 7: Large heap - build valid heap then break root
    std::vector<EmergencyPatient> heap7;
    for (int i = 1; i <= 100; i++) {
        heap7.push_back({"P" + std::to_string(i), i});
        heapifyUp(heap7, i - 1);
    }
    heap7[0].emergencyLevel = 1; // Break heap property
    auto before7 = heap7;
    heapifyDown(heap7, 0);
    check_size_invariant(before7.size(), heap7.size());
    check_elements_preserved(before7, heap7);
    check_max_heap_property(heap7);
    
    // Test 8: Invalid index (negative)
    std::vector<EmergencyPatient> heap8 = {{"P1", 5}, {"P2", 8}};
    auto before8 = heap8;
    heapifyDown(heap8, -1);
    check_size_invariant(before8.size(), heap8.size());
    
    // Test 9: Invalid index (out of bounds)
    std::vector<EmergencyPatient> heap9 = {{"P1", 5}, {"P2", 8}};
    auto before9 = heap9;
    heapifyDown(heap9, 100);
    check_size_invariant(before9.size(), heap9.size());
    
    // Test 10: Stress test - simulate extract-max operations
    std::vector<EmergencyPatient> heap10;
    for (int i = 0; i < 500; i++) {
        heap10.push_back({"P" + std::to_string(i), i % 50});
        heapifyUp(heap10, i);
    }
    
    for (int i = 0; i < 100; i++) {
        if (!heap10.empty()) {
            std::swap(heap10[0], heap10.back());
            heap10.pop_back();
            if (!heap10.empty()) {
                heapifyDown(heap10, 0);
                check_max_heap_property(heap10);
            }
        }
    }
    
    std::cout << "All heapifyDown strict tests passed!" << std::endl;
    return 0;
}
