#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <map>
#include <algorithm>

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
    // PROPERTY 2: Size increases by 1 after add
    // ========================================
    auto check_size_increase = [](size_t before, size_t after) {
        assert(after == before + 1 && "Size must increase by exactly 1");
    };
    
    // ========================================
    // PROPERTY 3: Added patient is in queue
    // ========================================
    auto check_patient_in_queue = [](const std::vector<EmergencyPatient>& queue,
                                     const EmergencyPatient& patient) {
        bool found = false;
        for (const auto& p : queue) {
            if (p.name == patient.name && p.emergencyLevel == patient.emergencyLevel) {
                found = true;
                break;
            }
        }
        assert(found && "Added patient must be in queue");
    };
    
    // ========================================
    // PROPERTY 4: Highest priority at root
    // ========================================
    auto check_max_at_root = [](const std::vector<EmergencyPatient>& queue) {
        if (queue.empty()) return;
        
        int maxLevel = queue[0].emergencyLevel;
        for (const auto& p : queue) {
            assert(maxLevel >= p.emergencyLevel && 
                   "Root must have highest priority");
        }
    };
    
    // ========================================
    // PROPERTY 5: getNext returns highest priority
    // ========================================
    auto check_returns_max = [](const std::vector<EmergencyPatient>& before,
                                const EmergencyPatient& returned) {
        int maxLevel = returned.emergencyLevel;
        for (const auto& p : before) {
            assert(maxLevel >= p.emergencyLevel && 
                   "Returned patient must have highest priority");
        }
    };
    
    // ========================================
    // PROPERTY 6: Size decreases by 1 after getNext
    // ========================================
    auto check_size_decrease = [](size_t before, size_t after) {
        assert(after == before - 1 && "Size must decrease by exactly 1");
    };
    
    // ========================================
    // PROPERTY 7: Monotonic extraction (non-increasing priorities)
    // ========================================
    auto check_monotonic_extraction = [](int prev, int current) {
        assert(prev >= current && 
               "Extracted priorities must be non-increasing");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Add to empty queue
    std::vector<EmergencyPatient> queue1;
    EmergencyPatient p1 = {"Patient1", 5};
    size_t size1_before = queue1.size();
    addEmergencyPatient(queue1, p1);
    check_size_increase(size1_before, queue1.size());
    check_patient_in_queue(queue1, p1);
    check_max_heap_property(queue1);
    check_max_at_root(queue1);
    assert(queue1[0].emergencyLevel == 5 && "Single element at root");
    
    // Test 2: Add multiple patients with different priorities
    std::vector<EmergencyPatient> queue2;
    EmergencyPatient p2a = {"P1", 3};
    EmergencyPatient p2b = {"P2", 8};
    EmergencyPatient p2c = {"P3", 5};
    
    addEmergencyPatient(queue2, p2a);
    check_max_heap_property(queue2);
    addEmergencyPatient(queue2, p2b);
    check_max_heap_property(queue2);
    addEmergencyPatient(queue2, p2c);
    check_max_heap_property(queue2);
    
    assert(queue2.size() == 3 && "All patients added");
    check_max_at_root(queue2);
    assert(queue2[0].emergencyLevel == 8 && "Highest priority at root");
    
    // Test 3: Add then extract - verify priority order
    std::vector<EmergencyPatient> queue3;
    addEmergencyPatient(queue3, {"P1", 3});
    addEmergencyPatient(queue3, {"P2", 8});
    addEmergencyPatient(queue3, {"P3", 5});
    addEmergencyPatient(queue3, {"P4", 10});
    
    auto before3 = queue3;
    auto next3a = getNextEmergencyPatient(queue3);
    check_returns_max(before3, next3a);
    check_size_decrease(before3.size(), queue3.size());
    check_max_heap_property(queue3);
    assert(next3a.emergencyLevel == 10 && "Highest priority extracted first");
    
    before3 = queue3;
    auto next3b = getNextEmergencyPatient(queue3);
    check_returns_max(before3, next3b);
    check_monotonic_extraction(next3a.emergencyLevel, next3b.emergencyLevel);
    assert(next3b.emergencyLevel == 8 && "Second highest extracted");
    
    // Test 4: Duplicate priorities
    std::vector<EmergencyPatient> queue4;
    for (int i = 0; i < 5; i++) {
        addEmergencyPatient(queue4, {"Patient" + std::to_string(i), 7});
        check_max_heap_property(queue4);
    }
    
    assert(queue4.size() == 5 && "All duplicates added");
    for (int i = 0; i < 5; i++) {
        auto p = getNextEmergencyPatient(queue4);
        assert(p.emergencyLevel == 7 && "All have same priority");
        check_max_heap_property(queue4);
    }
    assert(queue4.empty() && "All extracted");
    
    // Test 5: Large scale add/extract
    std::vector<EmergencyPatient> queue5;
    for (int i = 0; i < 100; i++) {
        addEmergencyPatient(queue5, {"P" + std::to_string(i), i % 10});
        check_max_heap_property(queue5);
    }
    
    int prevLevel = 10;
    while (!queue5.empty()) {
        auto p = getNextEmergencyPatient(queue5);
        check_monotonic_extraction(prevLevel, p.emergencyLevel);
        prevLevel = p.emergencyLevel;
        check_max_heap_property(queue5);
    }
    
    // Test 6: Interleaved add/extract
    std::vector<EmergencyPatient> queue6;
    addEmergencyPatient(queue6, {"P1", 5});
    addEmergencyPatient(queue6, {"P2", 8});
    
    auto p6a = getNextEmergencyPatient(queue6);
    assert(p6a.emergencyLevel == 8 && "Extract max");
    
    addEmergencyPatient(queue6, {"P3", 10});
    check_max_heap_property(queue6);
    
    auto p6b = getNextEmergencyPatient(queue6);
    assert(p6b.emergencyLevel == 10 && "New max extracted");
    
    auto p6c = getNextEmergencyPatient(queue6);
    assert(p6c.emergencyLevel == 5 && "Last element");
    assert(queue6.empty() && "Queue empty");
    
    // Test 7: Boundary priorities (0 and high values)
    std::vector<EmergencyPatient> queue7;
    addEmergencyPatient(queue7, {"Zero", 0});
    addEmergencyPatient(queue7, {"High", 100});
    addEmergencyPatient(queue7, {"Mid", 50});
    
    check_max_heap_property(queue7);
    assert(queue7[0].emergencyLevel == 100 && "Highest at root");
    
    auto p7a = getNextEmergencyPatient(queue7);
    assert(p7a.emergencyLevel == 100);
    auto p7b = getNextEmergencyPatient(queue7);
    assert(p7b.emergencyLevel == 50);
    auto p7c = getNextEmergencyPatient(queue7);
    assert(p7c.emergencyLevel == 0);
    
    // Test 8: Stress test - many operations
    std::vector<EmergencyPatient> queue8;
    for (int i = 0; i < 500; i++) {
        addEmergencyPatient(queue8, {"P" + std::to_string(i), i % 20});
        if (i % 3 == 0 && !queue8.empty()) {
            getNextEmergencyPatient(queue8);
        }
        check_max_heap_property(queue8);
    }
    
    // Extract all remaining
    int prev8 = 100;
    while (!queue8.empty()) {
        auto p = getNextEmergencyPatient(queue8);
        check_monotonic_extraction(prev8, p.emergencyLevel);
        prev8 = p.emergencyLevel;
    }
    
    // Test 9: Round-trip consistency
    std::vector<EmergencyPatient> queue9;
    std::map<int, int> addedCounts;
    
    for (int i = 0; i < 50; i++) {
        int level = i % 10;
        addEmergencyPatient(queue9, {"P" + std::to_string(i), level});
        addedCounts[level]++;
    }
    
    std::map<int, int> extractedCounts;
    while (!queue9.empty()) {
        auto p = getNextEmergencyPatient(queue9);
        extractedCounts[p.emergencyLevel]++;
    }
    
    assert(addedCounts == extractedCounts && 
           "All added patients must be extracted");
    
    std::cout << "All addEmergencyPatient/getNextEmergencyPatient strict tests passed!" << std::endl;
    return 0;
}
