#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <algorithm>

void test_heapifyUp() {
    // Test empty heap - edge case
    std::vector<EmergencyPatient> emptyHeap;
    heapifyUp(emptyHeap, 0); // Should not crash
    
    // Test single element
    std::vector<EmergencyPatient> singleHeap = {{"Patient1", 5}};
    heapifyUp(singleHeap, 0);
    assert(singleHeap[0].emergencyLevel == 5);
    
    // Test already correct heap
    std::vector<EmergencyPatient> correctHeap = {{"Patient1", 10}, {"Patient2", 8}, {"Patient3", 5}};
    heapifyUp(correctHeap, 2);
    assert(correctHeap[0].emergencyLevel == 10);
    
    // Test basic heapify up
    std::vector<EmergencyPatient> heap = {{"Patient1", 5}, {"Patient2", 8}, {"Patient3", 3}, {"Patient4", 10}};
    heapifyUp(heap, 3);
    assert(heap[0].emergencyLevel == 10);
    assert(heap[0].name == "Patient4");
    
    // Test large dataset with random priorities
    std::vector<EmergencyPatient> largeHeap;
    for(int i = 0; i < 100; i++) {
        largeHeap.push_back({"Patient" + std::to_string(i), i % 15});
    }
    largeHeap.push_back({"Critical", 20});
    heapifyUp(largeHeap, 100);
    assert(largeHeap[0].emergencyLevel == 20);
    
    // Test boundary conditions - invalid indices
    std::vector<EmergencyPatient> testHeap = {{"Patient1", 5}, {"Patient2", 8}};
    heapifyUp(testHeap, -1); // Should handle gracefully
    heapifyUp(testHeap, 100); // Should handle gracefully
    
    // Test duplicate priorities
    std::vector<EmergencyPatient> dupHeap = {{"Patient1", 5}, {"Patient2", 5}, {"Patient3", 5}, {"Patient4", 10}};
    heapifyUp(dupHeap, 3);
    assert(dupHeap[0].emergencyLevel == 10);
    
    std::cout << "test_heapifyUp passed!" << std::endl;
}

int main() {
    test_heapifyUp();
    return 0;
}