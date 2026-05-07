#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void test_searchBySpecialization() {
    // Test empty tree
    BSTNode* root = nullptr;
    auto emptyResult = searchBySpecialization(root, "Cardiology");
    assert(emptyResult.empty());
    
    // Test single doctor
    Doctor d1 = {"Dr. Smith", "Cardiology", 1, true};
    root = insertDoctor(root, d1);
    auto result1 = searchBySpecialization(root, "Cardiology");
    assert(result1.size() == 1);
    assert(result1[0].name == "Dr. Smith");
    
    // Test multiple doctors same specialization
    Doctor d2 = {"Dr. Jones", "Cardiology", 2, true};
    root = insertDoctor(root, d2);
    auto result2 = searchBySpecialization(root, "Cardiology");
    assert(result2.size() == 2);
    
    // Test different specializations
    Doctor d3 = {"Dr. Brown", "Neurology", 3, true};
    Doctor d4 = {"Dr. White", "Orthopedics", 4, true};
    root = insertDoctor(root, d3);
    root = insertDoctor(root, d4);
    
    auto neuroResult = searchBySpecialization(root, "Neurology");
    assert(neuroResult.size() == 1);
    assert(neuroResult[0].specialization == "Neurology");
    
    // Test non-existent specialization
    auto notFound = searchBySpecialization(root, "Dermatology");
    assert(notFound.empty());
    
    // Test case sensitivity
    auto caseTest = searchBySpecialization(root, "cardiology");
    assert(caseTest.empty()); // Should be case-sensitive
    
    // Test large tree
    BSTNode* largeRoot = nullptr;
    for(int i = 0; i < 50; i++) {
        Doctor d = {"Dr" + std::to_string(i), "Spec" + std::to_string(i % 10), i, true};
        largeRoot = insertDoctor(largeRoot, d);
    }
    auto largeResult = searchBySpecialization(largeRoot, "Spec5");
    assert(largeResult.size() == 5);
    
    std::cout << "test_searchBySpecialization passed!" << std::endl;
}

int main() {
    test_searchBySpecialization();
    return 0;
}
