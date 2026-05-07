#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>

void cleanupBST(BSTNode* root) {
    if (!root) return;
    cleanupBST(root->left);
    cleanupBST(root->right);
    delete root;
}

void test_insertDoctor() {
    // Test null root insertion
    BSTNode* root = nullptr;
    Doctor doc1 = {"Dr. Smith", "Cardiology", 1, true};
    root = insertDoctor(root, doc1);
    assert(root != nullptr);
    assert(root->doctors.front().specialization == "Cardiology");
    
    // Test duplicate specialization
    Doctor doc2 = {"Dr. Jones", "Cardiology", 2, false};
    root = insertDoctor(root, doc2);
    assert(root->doctors.size() == 2);
    
    // Test different specializations (BST structure)
    Doctor doc3 = {"Dr. Brown", "Neurology", 3, true};
    Doctor doc4 = {"Dr. White", "Anesthesia", 4, true};
    root = insertDoctor(root, doc3);
    root = insertDoctor(root, doc4);
    
    assert(root->right != nullptr); // Neurology > Cardiology
    assert(root->left != nullptr);  // Anesthesia < Cardiology
    
    // Test large dataset insertion
    std::vector<std::string> specializations = {
        "Dermatology", "Emergency", "Gastroenterology", "Hematology",
        "Immunology", "Nephrology", "Oncology", "Pathology",
        "Psychiatry", "Radiology", "Surgery", "Urology"
    };
    
    for(int i = 0; i < specializations.size(); i++) {
        Doctor doc = {"Dr. Test" + std::to_string(i), specializations[i], 100 + i, i % 2 == 0};
        root = insertDoctor(root, doc);
    }
    
    // Test empty doctor name
    Doctor emptyDoc = {"", "Cardiology", 999, true};
    root = insertDoctor(root, emptyDoc);
    
    // Test negative ID
    Doctor negativeIdDoc = {"Dr. Negative", "Cardiology", -1, true};
    root = insertDoctor(root, negativeIdDoc);
    
    // Test very long specialization name
    std::string longSpec(1000, 'A');
    Doctor longSpecDoc = {"Dr. Long", longSpec, 1000, true};
    root = insertDoctor(root, longSpecDoc);
    
    // Memory cleanup test
    cleanupBST(root);
    
    // Test insertion after cleanup (new tree)
    root = nullptr;
    root = insertDoctor(root, doc1);
    assert(root != nullptr);
    
    cleanupBST(root);
    std::cout << "test_insertDoctor passed!" << std::endl;
}

int main() {
    test_insertDoctor();
    return 0;
}