#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void test_addEmergencyPatient() {
    // Test adding to empty queue
    std::vector<EmergencyPatient> queue;
    EmergencyPatient p1 = {"Patient1", 5};
    addEmergencyPatient(queue, p1);
    assert(queue.size() == 1);
    assert(queue[0].name == "Patient1");
    assert(queue[0].emergencyLevel == 5);
    
    // Test adding multiple patients
    EmergencyPatient p2 = {"Patient2", 8};
    EmergencyPatient p3 = {"Patient3", 3};
    addEmergencyPatient(queue, p2);
    addEmergencyPatient(queue, p3);
    assert(queue.size() == 3);
    
    // Test max heap property maintained
    assert(queue[0].emergencyLevel >= queue[1].emergencyLevel);
    assert(queue[0].emergencyLevel >= queue[2].emergencyLevel);
    
    // Test adding high priority patient
    EmergencyPatient critical = {"Critical", 10};
    addEmergencyPatient(queue, critical);
    assert(queue[0].emergencyLevel == 10);
    assert(queue[0].name == "Critical");
    
    // Test adding low priority patient
    EmergencyPatient low = {"Low", 1};
    addEmergencyPatient(queue, low);
    assert(queue[0].emergencyLevel == 10); // Max still at top
    
    // Test large number of patients
    std::vector<EmergencyPatient> largeQueue;
    for(int i = 0; i < 100; i++) {
        EmergencyPatient p = {"Patient" + std::to_string(i), i % 10};
        addEmergencyPatient(largeQueue, p);
    }
    assert(largeQueue.size() == 100);
    assert(largeQueue[0].emergencyLevel == 9); // Max priority
    
    // Test duplicate priorities
    std::vector<EmergencyPatient> dupQueue;
    for(int i = 0; i < 5; i++) {
        EmergencyPatient p = {"Patient" + std::to_string(i), 5};
        addEmergencyPatient(dupQueue, p);
    }
    assert(dupQueue.size() == 5);
    assert(dupQueue[0].emergencyLevel == 5);
    
    // Test boundary emergency levels
    std::vector<EmergencyPatient> boundaryQueue;
    addEmergencyPatient(boundaryQueue, {"Zero", 0});
    addEmergencyPatient(boundaryQueue, {"Max", 100});
    assert(boundaryQueue[0].emergencyLevel == 100);
    
    std::cout << "test_addEmergencyPatient passed!" << std::endl;
}

int main() {
    test_addEmergencyPatient();
    return 0;
}
