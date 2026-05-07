#ifndef HOSPITAL_FUNCTIONS_H
#define HOSPITAL_FUNCTIONS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <list>

// Data structures
struct Doctor {
    std::string name;
    std::string specialization;
    int id;
    bool available;
};

struct EmergencyPatient {
    std::string name;
    int emergencyLevel;
};

struct BSTNode {
    std::list<Doctor> doctors;
    BSTNode* left;
    BSTNode* right;
    
    BSTNode() : left(nullptr), right(nullptr) {}
};

// Easy functions (20%)
void heapifyUp(std::vector<EmergencyPatient>& heap, int index);
void heapifyDown(std::vector<EmergencyPatient>& heap, int index);
int findMaxId(const std::vector<Doctor>& doctors);
bool validateAge(int age);

// Medium functions (50%)
BSTNode* insertDoctor(BSTNode* root, const Doctor& doctor);
std::vector<Doctor> searchBySpecialization(BSTNode* root, const std::string& specialization);
void addEmergencyPatient(std::vector<EmergencyPatient>& queue, const EmergencyPatient& patient);
EmergencyPatient getNextEmergencyPatient(std::vector<EmergencyPatient>& queue);
std::pair<std::vector<std::string>, int> dijkstraShortestPath(
    const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
    const std::string& source, const std::string& target);
bool isDoctorAvailable(const std::vector<Doctor>& doctors, const std::string& specialization);
void updateDoctorAvailability(std::vector<Doctor>& doctors, int doctorId, bool available);
std::vector<Doctor> getDoctorsBySpecialization(const std::vector<Doctor>& doctors, const std::string& specialization);
int calculateDistance(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                     const std::string& from, const std::string& to);
bool isEmergencyQueueEmpty(const std::vector<EmergencyPatient>& queue);

// Hard functions (30%)
int processPatientWorkflow(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue,
                          const std::string& patientName, const std::string& requiredSpecialization,
                          int emergencyLevel);
std::string dispatchAmbulance(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& cityGraph,
                             const std::string& hospitalLocation, const std::string& emergencyLocation);
void manageEmergencyOPD(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue);
int bookAppointment(std::vector<Doctor>& doctors, const std::string& patientName,
                   const std::string& specialization, const std::string& preferredTime);
std::vector<std::string> getOptimalRoute(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                                        const std::vector<std::string>& locations);
int managePatientLifecycle(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue,
                          const std::string& patientName, int age, const std::string& symptoms);

#endif // HOSPITAL_FUNCTIONS_H