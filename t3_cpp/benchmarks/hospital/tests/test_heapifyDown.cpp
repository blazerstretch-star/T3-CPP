#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>

void test_heapifyDown() {
    // Test empty heap - edge case
    std::vector<EmergencyPatient> emptyHeap;
    heapifyDown(emptyHeap, 0); // Should not crash
    
    // Test single element
    std::vector<EmergencyPatient> singleHeap = {{"Patient1", 5}};
    heapifyDown(singleHeap, 0);
    assert(singleHeap[0].emergencyLevel == 5);
    
    // Test basic heapify down - root needs to move down
    std::vector<EmergencyPatient> heap = {{"Patient1", 3}, {"Patient2", 8}, {"Patient3", 5}};
    heapifyDown(heap, 0);
    assert(heap[0].emergencyLevel == 8);
    
    // Test already correct max heap
    std::vector<EmergencyPatient> correctHeap = {{"Patient1", 10}, {"Patient2", 8}, {"Patient3", 5}};
    heapifyDown(correctHeap, 0);
    assert(correctHeap[0].emergencyLevel == 10);
    
    // Test heapify from middle
    std::vector<EmergencyPatient> midHeap = {{"P1", 10}, {"P2", 3}, {"P3", 8}, {"P4", 7}, {"P5", 6}};
    heapifyDown(midHeap, 1);
    assert(midHeap[1].emergencyLevel >= midHeap[3].emergencyLevel);
    assert(midHeap[1].emergencyLevel >= midHeap[4].emergencyLevel);
    
    // Test large heap
    std::vector<EmergencyPatient> largeHeap = {{"P0", 99}};
    for(int i = 1; i < 100; i++) {
        largeHeap.push_back({"Patient" + std::to_string(i), i});
        heapifyUp(largeHeap, i); // Build valid heap
    }
    largeHeap[0].emergencyLevel = 1; // Break heap property at root
    heapifyDown(largeHeap, 0);
    assert(largeHeap[0].emergencyLevel >= largeHeap[1].emergencyLevel);
    assert(largeHeap[0].emergencyLevel >= largeHeap[2].emergencyLevel);
    
    // Test boundary - invalid indices
    std::vector<EmergencyPatient> testHeap = {{"Patient1", 5}, {"Patient2", 8}};
    heapifyDown(testHeap, -1); // Should handle gracefully
    heapifyDown(testHeap, 100); // Should handle gracefully
    
    // Test duplicate priorities
    std::vector<EmergencyPatient> dupHeap = {{"P1", 5}, {"P2", 10}, {"P3", 10}};
    heapifyDown(dupHeap, 0);
    assert(dupHeap[0].emergencyLevel == 10);
    
    std::cout << "test_heapifyDown passed!" << std::endl;
}

int main() {
    test_heapifyDown();
    return 0;
}
