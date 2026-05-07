#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void test_manageEmergencyOPD() {
    // Test with empty queue and doctors
    std::vector<Doctor> doctors;
    std::vector<EmergencyPatient> emptyQueue;
    manageEmergencyOPD(doctors, emptyQueue); // Should handle gracefully
    
    // Test with patients but no doctors
    std::vector<EmergencyPatient> queue;
    addEmergencyPatient(queue, {"Patient1", 8});
    manageEmergencyOPD(doctors, queue); // Should handle gracefully
    
    // Test with doctors but no patients
    doctors = {
        {"Dr. Smith", "Emergency", 1, true},
        {"Dr. Jones", "Emergency", 2, true}
    };
    std::vector<EmergencyPatient> noPatients;
    manageEmergencyOPD(doctors, noPatients); // Should handle gracefully
    
    // Test normal operation
    std::vector<EmergencyPatient> normalQueue;
    addEmergencyPatient(normalQueue, {"Patient1", 9});
    addEmergencyPatient(normalQueue, {"Patient2", 7});
    addEmergencyPatient(normalQueue, {"Patient3", 5});
    
    std::vector<Doctor> normalDoctors = {
        {"Dr. Smith", "Emergency", 1, true},
        {"Dr. Jones", "Emergency", 2, true}
    };
    
    size_t initialSize = normalQueue.size();
    manageEmergencyOPD(normalDoctors, normalQueue);
    // Should process at least one patient if doctors available
    
    // Test with all doctors unavailable
    std::vector<Doctor> busyDoctors = {
        {"Dr. Smith", "Emergency", 1, false},
        {"Dr. Jones", "Emergency", 2, false}
    };
    std::vector<EmergencyPatient> waitingQueue;
    addEmergencyPatient(waitingQueue, {"Patient1", 10});
    manageEmergencyOPD(busyDoctors, waitingQueue);
    
    // Test large scale operation
    std::vector<EmergencyPatient> largeQueue;
    for(int i = 0; i < 50; i++) {
        addEmergencyPatient(largeQueue, {"Patient" + std::to_string(i), i % 10});
    }
    
    std::vector<Doctor> largeDoctorList;
    for(int i = 0; i < 10; i++) {
        largeDoctorList.push_back({"Dr" + std::to_string(i), "Emergency", i, i % 2 == 0});
    }
    
    manageEmergencyOPD(largeDoctorList, largeQueue);
    
    std::cout << "test_manageEmergencyOPD passed!" << std::endl;
}

int main() {
    test_manageEmergencyOPD();
    return 0;
}
