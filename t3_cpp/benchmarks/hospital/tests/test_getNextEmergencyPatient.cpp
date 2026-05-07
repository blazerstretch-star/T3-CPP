#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void test_getNextEmergencyPatient() {
    // Test with single patient
    std::vector<EmergencyPatient> queue;
    EmergencyPatient p1 = {"Patient1", 5};
    addEmergencyPatient(queue, p1);
    
    auto next = getNextEmergencyPatient(queue);
    assert(next.name == "Patient1");
    assert(next.emergencyLevel == 5);
    assert(queue.empty());
    
    // Test priority ordering
    EmergencyPatient p2 = {"Patient2", 3};
    EmergencyPatient p3 = {"Patient3", 8};
    EmergencyPatient p4 = {"Patient4", 5};
    addEmergencyPatient(queue, p2);
    addEmergencyPatient(queue, p3);
    addEmergencyPatient(queue, p4);
    
    auto first = getNextEmergencyPatient(queue);
    assert(first.emergencyLevel == 8);
    assert(queue.size() == 2);
    
    auto second = getNextEmergencyPatient(queue);
    assert(second.emergencyLevel == 5);
    assert(queue.size() == 1);
    
    auto third = getNextEmergencyPatient(queue);
    assert(third.emergencyLevel == 3);
    assert(queue.empty());
    
    // Test large queue
    std::vector<EmergencyPatient> largeQueue;
    for(int i = 0; i < 50; i++) {
        EmergencyPatient p = {"Patient" + std::to_string(i), i % 10};
        addEmergencyPatient(largeQueue, p);
    }
    
    int prevLevel = 10;
    while(!largeQueue.empty()) {
        auto patient = getNextEmergencyPatient(largeQueue);
        assert(patient.emergencyLevel <= prevLevel);
        prevLevel = patient.emergencyLevel;
    }
    
    // Test duplicate priorities
    std::vector<EmergencyPatient> dupQueue;
    for(int i = 0; i < 5; i++) {
        addEmergencyPatient(dupQueue, {"Patient" + std::to_string(i), 7});
    }
    
    for(int i = 0; i < 5; i++) {
        auto p = getNextEmergencyPatient(dupQueue);
        assert(p.emergencyLevel == 7);
    }
    assert(dupQueue.empty());
    
    std::cout << "test_getNextEmergencyPatient passed!" << std::endl;
}

int main() {
    test_getNextEmergencyPatient();
    return 0;
}
