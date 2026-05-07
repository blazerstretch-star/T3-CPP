#include "hospital_functions.h"
#include <algorithm>
#include <limits>
#include <queue>
#include <iostream>
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


// Easy functions (20%)
void heapifyUp(std::vector<EmergencyPatient>& heap, int index) {
    // FUNCTION_ID: hospital_func001 - START
    while (index > 0) {
        int parentIndex = (index - 1) / 2;
        if (heap[index].emergencyLevel > heap[parentIndex].emergencyLevel) {
            std::swap(heap[index], heap[parentIndex]);
            index = parentIndex;
        } else {
            break;
        }
    }
    // FUNCTION_ID: hospital_func001 - END
}

void heapifyDown(std::vector<EmergencyPatient>& heap, int index) {
    // FUNCTION_ID: hospital_func002 - START
    int leftChild = 2 * index + 1;
    int rightChild = 2 * index + 2;
    int highestPriority = index;

    if (leftChild < heap.size() && heap[leftChild].emergencyLevel > heap[highestPriority].emergencyLevel) {
        highestPriority = leftChild;
    }

    if (rightChild < heap.size() && heap[rightChild].emergencyLevel > heap[highestPriority].emergencyLevel) {
        highestPriority = rightChild;
    }

    if (highestPriority != index) {
        std::swap(heap[index], heap[highestPriority]);
        heapifyDown(heap, highestPriority);
    }
    // FUNCTION_ID: hospital_func002 - END
}

int findMaxId(const std::vector<Doctor>& doctors) {
    // FUNCTION_ID: hospital_func003 - START
    int maxId = 0;
    for (const auto& doctor : doctors) {
        if (doctor.id > maxId) {
            maxId = doctor.id;
        }
    }
    return maxId;
    // FUNCTION_ID: hospital_func003 - END
}

bool validateAge(int age) {
    // FUNCTION_ID: hospital_func004 - START
    return age >= 0 && age <= 150;
    // FUNCTION_ID: hospital_func004 - END
}

// Medium functions (50%)
BSTNode* insertDoctor(BSTNode* root, const Doctor& doctor) {
    // FUNCTION_ID: hospital_func005 - START
    if (root == nullptr) {
        BSTNode* newNode = new BSTNode();
        newNode->doctors.push_back(doctor);
        return newNode;
    }

    if (doctor.specialization < root->doctors.front().specialization) {
        root->left = insertDoctor(root->left, doctor);
    } else if (doctor.specialization > root->doctors.front().specialization) {
        root->right = insertDoctor(root->right, doctor);
    } else {
        root->doctors.push_back(doctor);
    }

    return root;
    // FUNCTION_ID: hospital_func005 - END
}

std::vector<Doctor> searchBySpecialization(BSTNode* root, const std::string& specialization) {
    // FUNCTION_ID: hospital_func006 - START
    std::vector<Doctor> result;
    BSTNode* current = root;
    
    while (current != nullptr) {
        if (specialization < current->doctors.front().specialization) {
            current = current->left;
        } else if (specialization > current->doctors.front().specialization) {
            current = current->right;
        } else {
            for (const auto& doctor : current->doctors) {
                result.push_back(doctor);
            }
            break;
        }
    }
    
    return result;
    // FUNCTION_ID: hospital_func006 - END
}

void addEmergencyPatient(std::vector<EmergencyPatient>& queue, const EmergencyPatient& patient) {
    // FUNCTION_ID: hospital_func007 - START
    queue.push_back(patient);
    heapifyUp(queue, queue.size() - 1);
    // FUNCTION_ID: hospital_func007 - END
}

EmergencyPatient getNextEmergencyPatient(std::vector<EmergencyPatient>& queue) {
    // FUNCTION_ID: hospital_func008 - START
    if (queue.empty()) {
        throw std::runtime_error("Emergency queue is empty");
    }
    
    EmergencyPatient nextPatient = queue[0];
    std::swap(queue[0], queue.back());
    queue.pop_back();
    
    if (!queue.empty()) {
        heapifyDown(queue, 0);
    }
    
    return nextPatient;
    // FUNCTION_ID: hospital_func008 - END
}

std::pair<std::vector<std::string>, int> dijkstraShortestPath(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph, const std::string& source, const std::string& target) {
    // FUNCTION_ID: hospital_func009 - START
    std::unordered_map<std::string, bool> visited;
    std::unordered_map<std::string, int> distance;
    std::unordered_map<std::string, std::string> previous;

    for (const auto& vertex : graph) {
        visited[vertex.first] = false;
        distance[vertex.first] = std::numeric_limits<int>::max();
        previous[vertex.first] = "";
    }
    
    distance[source] = 0;

    while (!visited[target]) {
        std::string selectedNode;
        int minDistance = std::numeric_limits<int>::max();

        for (const auto& vertex : graph) {
            if (!visited[vertex.first] && distance[vertex.first] < minDistance) {
                minDistance = distance[vertex.first];
                selectedNode = vertex.first;
            }
        }

        visited[selectedNode] = true;

        for (const auto& neighbor : graph.at(selectedNode)) {
            if (!visited[neighbor.first]) {
                int alt = distance[selectedNode] + neighbor.second;
                if (alt < distance[neighbor.first]) {
                    distance[neighbor.first] = alt;
                    previous[neighbor.first] = selectedNode;
                }
            }
        }
    }

    std::vector<std::string> path;
    for (std::string vertex = target; !vertex.empty(); vertex = previous[vertex]) {
        path.push_back(vertex);
    }
    
    std::reverse(path.begin(), path.end());
    return std::make_pair(path, distance[target]);
    // FUNCTION_ID: hospital_func009 - END
}

bool isDoctorAvailable(const std::vector<Doctor>& doctors, const std::string& specialization) {
    // FUNCTION_ID: hospital_func010 - START
    for (const auto& doctor : doctors) {
        if (doctor.specialization == specialization && doctor.available) {
            return true;
        }
    }
    return false;
    // FUNCTION_ID: hospital_func010 - END
}

void updateDoctorAvailability(std::vector<Doctor>& doctors, int doctorId, bool available) {
    // FUNCTION_ID: hospital_func011 - START
    for (auto& doctor : doctors) {
        if (doctor.id == doctorId) {
            doctor.available = available;
            break;
        }
    }
    // FUNCTION_ID: hospital_func011 - END
}

std::vector<Doctor> getDoctorsBySpecialization(const std::vector<Doctor>& doctors, const std::string& specialization) {
    // FUNCTION_ID: hospital_func012 - START
    std::vector<Doctor> result;
    for (const auto& doctor : doctors) {
        if (doctor.specialization == specialization) {
            result.push_back(doctor);
        }
    }
    return result;
    // FUNCTION_ID: hospital_func012 - END
}

int calculateDistance(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                     const std::string& from, const std::string& to) {
    // FUNCTION_ID: hospital_func013 - START
    auto result = dijkstraShortestPath(graph, from, to);
    return result.second;
    // FUNCTION_ID: hospital_func013 - END
}

bool isEmergencyQueueEmpty(const std::vector<EmergencyPatient>& queue) {
    // FUNCTION_ID: hospital_func014 - START
    return queue.empty();
    // FUNCTION_ID: hospital_func014 - END
}

// Hard functions (30%)
int processPatientWorkflow(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue,
                          const std::string& patientName, const std::string& requiredSpecialization,
                          int emergencyLevel) {
    // FUNCTION_ID: hospital_func015 - START
    if (doctors.empty()) return -1; // No doctors available
    if (!validateAge(25)) return -1; // Assume default age validation
    
    if (emergencyLevel > 7) {
        EmergencyPatient patient = {patientName, emergencyLevel};
        addEmergencyPatient(emergencyQueue, patient);
        
        if (isDoctorAvailable(doctors, requiredSpecialization)) {
            auto availableDoctors = getDoctorsBySpecialization(doctors, requiredSpecialization);
            for (auto& doctor : availableDoctors) {
                if (doctor.available) {
                    updateDoctorAvailability(doctors, doctor.id, false);
                    return doctor.id;
                }
            }
        }
        return 0; // Emergency registered but no doctor available
    } else {
        return bookAppointment(doctors, patientName, requiredSpecialization, "next_available");
    }
    // FUNCTION_ID: hospital_func015 - END
}

std::string dispatchAmbulance(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& cityGraph,
                             const std::string& hospitalLocation, const std::string& emergencyLocation) {
    // FUNCTION_ID: hospital_func016 - START
    auto result = dijkstraShortestPath(cityGraph, hospitalLocation, emergencyLocation);
    
    if (result.second == std::numeric_limits<int>::max()) {
        return "No route available";
    }
    
    std::string route = "Ambulance dispatched: ";
    for (size_t i = 0; i < result.first.size(); ++i) {
        route += result.first[i];
        if (i < result.first.size() - 1) {
            route += " -> ";
        }
    }
    route += " (Distance: " + std::to_string(result.second) + " km)";
    
    return route;
    // FUNCTION_ID: hospital_func016 - END
}

void manageEmergencyOPD(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue) {
    // FUNCTION_ID: hospital_func017 - START
    while (!isEmergencyQueueEmpty(emergencyQueue)) {
        EmergencyPatient nextPatient = getNextEmergencyPatient(emergencyQueue);
        
        bool doctorFound = false;
        for (auto& doctor : doctors) {
            if (doctor.specialization == "Emergency" && doctor.available) {
                updateDoctorAvailability(doctors, doctor.id, false);
                std::cout << "Processing " << nextPatient.name << " with Dr. " << doctor.name << std::endl;
                doctorFound = true;
                break;
            }
        }
        
        if (!doctorFound) {
            addEmergencyPatient(emergencyQueue, nextPatient);
            break;
        }
    }
    // FUNCTION_ID: hospital_func017 - END
}

int bookAppointment(std::vector<Doctor>& doctors, const std::string& patientName,
                   const std::string& specialization, const std::string& preferredTime) {
    // FUNCTION_ID: hospital_func018 - START
    auto availableDoctors = getDoctorsBySpecialization(doctors, specialization);
    
    if (availableDoctors.empty()) {
        return -1; // No doctors with required specialization
    }
    
    for (const auto& doctor : availableDoctors) {
        if (doctor.available) {
            return doctor.id; // Return doctor ID for appointment
        }
    }
    
    return 0; // Doctors exist but none available
    // FUNCTION_ID: hospital_func018 - END
}

std::vector<std::string> getOptimalRoute(const std::unordered_map<std::string, std::unordered_map<std::string, int>>& graph,
                                        const std::vector<std::string>& locations) {
    // FUNCTION_ID: hospital_func019 - START
    if (locations.size() < 2) {
        return locations;
    }
    
    std::vector<std::string> optimalRoute;
    optimalRoute.push_back(locations[0]);
    
    for (size_t i = 1; i < locations.size(); ++i) {
        auto pathResult = dijkstraShortestPath(graph, locations[i-1], locations[i]);
        
        for (size_t j = 1; j < pathResult.first.size(); ++j) {
            optimalRoute.push_back(pathResult.first[j]);
        }
    }
    
    return optimalRoute;
    // FUNCTION_ID: hospital_func019 - END
}

int managePatientLifecycle(std::vector<Doctor>& doctors, std::vector<EmergencyPatient>& emergencyQueue,
                          const std::string& patientName, int age, const std::string& symptoms) {
    // FUNCTION_ID: hospital_func020 - START
    if (doctors.empty()) {
        return -1; // No doctors available
    }
    
    if (!validateAge(age)) {
        return -1; // Invalid age
    }
    
    std::string requiredSpecialization = "General";
    int emergencyLevel = 1;
    
    if (symptoms.find("chest pain") != std::string::npos) {
        requiredSpecialization = "Cardiology";
        emergencyLevel = 9;
    } else if (symptoms.find("fracture") != std::string::npos) {
        requiredSpecialization = "Orthopedics";
        emergencyLevel = 6;
    } else if (symptoms.find("fever") != std::string::npos) {
        requiredSpecialization = "General";
        emergencyLevel = 3;
    }
    
    return processPatientWorkflow(doctors, emergencyQueue, patientName, requiredSpecialization, emergencyLevel);
    // FUNCTION_ID: hospital_func020 - END
}