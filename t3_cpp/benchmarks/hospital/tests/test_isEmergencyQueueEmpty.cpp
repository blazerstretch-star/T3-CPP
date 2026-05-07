#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void test_isEmergencyQueueEmpty() {
    // Test empty queue
    std::vector<EmergencyPatient> queue;
    assert(isEmergencyQueueEmpty(queue) == true);
    
    // Test after adding one patient
    EmergencyPatient p1 = {"Patient1", 5};
    addEmergencyPatient(queue, p1);
    assert(isEmergencyQueueEmpty(queue) == false);
    
    // Test after removing patient
    getNextEmergencyPatient(queue);
    assert(isEmergencyQueueEmpty(queue) == true);
    
    // Test with multiple patients
    for(int i = 0; i < 10; i++) {
        addEmergencyPatient(queue, {"Patient" + std::to_string(i), i});
    }
    assert(isEmergencyQueueEmpty(queue) == false);
    
    // Test removing all patients
    while(!isEmergencyQueueEmpty(queue)) {
        getNextEmergencyPatient(queue);
    }
    assert(isEmergencyQueueEmpty(queue) == true);
    
    // Test alternating add/remove
    addEmergencyPatient(queue, {"P1", 5});
    assert(isEmergencyQueueEmpty(queue) == false);
    getNextEmergencyPatient(queue);
    assert(isEmergencyQueueEmpty(queue) == true);
    addEmergencyPatient(queue, {"P2", 8});
    assert(isEmergencyQueueEmpty(queue) == false);
    
    // Test large queue
    std::vector<EmergencyPatient> largeQueue;
    for(int i = 0; i < 1000; i++) {
        addEmergencyPatient(largeQueue, {"Patient" + std::to_string(i), i % 10});
    }
    assert(isEmergencyQueueEmpty(largeQueue) == false);
    
    std::cout << "test_isEmergencyQueueEmpty passed!" << std::endl;
}

int main() {
    test_isEmergencyQueueEmpty();
    return 0;
}
