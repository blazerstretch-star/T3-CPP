#include "../src/hospital_functions.h"
#include <cassert>
#include <iostream>
#include <functional>
#include <set>

// Helper to cleanup BST
void cleanupBST(BSTNode* root) {
    if (!root) return;
    cleanupBST(root->left);
    cleanupBST(root->right);
    delete root;
}

int main() {
    // ========================================
    // PROPERTY 1: BST ordering property
    // ========================================
    auto check_bst_property = [](BSTNode* root) {
        std::function<void(BSTNode*, std::string, std::string)> validate = 
            [&](BSTNode* node, std::string minSpec, std::string maxSpec) {
            if (!node) return;
            
            std::string nodeSpec = node->doctors.front().specialization;
            
            if (!minSpec.empty()) {
                assert(nodeSpec >= minSpec && 
                       "Node specialization must be >= all left ancestors");
            }
            if (!maxSpec.empty()) {
                assert(nodeSpec <= maxSpec && 
                       "Node specialization must be <= all right ancestors");
            }
            
            if (node->left) {
                std::string leftSpec = node->left->doctors.front().specialization;
                assert(leftSpec < nodeSpec && 
                       "Left child specialization must be < parent");
                validate(node->left, minSpec, nodeSpec);
            }
            
            if (node->right) {
                std::string rightSpec = node->right->doctors.front().specialization;
                assert(rightSpec > nodeSpec && 
                       "Right child specialization must be > parent");
                validate(node->right, nodeSpec, maxSpec);
            }
        };
        
        validate(root, "", "");
    };
    
    // ========================================
    // PROPERTY 2: All doctors preserved
    // ========================================
    std::function<int(BSTNode*)> count_doctors = [&](BSTNode* root) -> int {
        if (!root) return 0;
        int count = root->doctors.size();
        count += count_doctors(root->left);
        count += count_doctors(root->right);
        return count;
    };
    
    // ========================================
    // PROPERTY 3: Same specialization doctors in same node
    // ========================================
    auto check_specialization_grouping = [](BSTNode* root) {
        std::function<void(BSTNode*)> validate = [&](BSTNode* node) {
            if (!node) return;
            
            // All doctors in node must have same specialization
            if (!node->doctors.empty()) {
                std::string spec = node->doctors.front().specialization;
                for (const auto& doc : node->doctors) {
                    assert(doc.specialization == spec && 
                           "All doctors in node must have same specialization");
                }
            }
            
            validate(node->left);
            validate(node->right);
        };
        
        validate(root);
    };
    
    // ========================================
    // PROPERTY 4: Doctor findable after insertion
    // ========================================
    std::function<bool(BSTNode*, const Doctor&)> can_find_doctor = 
        [&](BSTNode* root, const Doctor& doctor) -> bool {
        if (!root) return false;
        
        for (const auto& d : root->doctors) {
            if (d.id == doctor.id && d.name == doctor.name) {
                return true;
            }
        }
        
        return can_find_doctor(root->left, doctor) || can_find_doctor(root->right, doctor);
    };
    
    // ========================================
    // PROPERTY 5: In-order traversal is sorted
    // ========================================
    auto check_inorder_sorted = [](BSTNode* root) {
        std::vector<std::string> inorder;
        std::function<void(BSTNode*)> traverse = [&](BSTNode* node) {
            if (!node) return;
            traverse(node->left);
            inorder.push_back(node->doctors.front().specialization);
            traverse(node->right);
        };
        
        traverse(root);
        
        for (size_t i = 1; i < inorder.size(); ++i) {
            assert(inorder[i - 1] < inorder[i] && 
                   "In-order traversal must be sorted");
        }
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Insert into null root
    BSTNode* root1 = nullptr;
    Doctor doc1 = {"Dr. Smith", "Cardiology", 1, true};
    root1 = insertDoctor(root1, doc1);
    assert(root1 != nullptr && "Root must not be null after insertion");
    assert(root1->doctors.size() == 1 && "Root must contain one doctor");
    assert(can_find_doctor(root1, doc1) && "Inserted doctor must be findable");
    check_bst_property(root1);
    check_specialization_grouping(root1);
    cleanupBST(root1);
    
    // Test 2: Insert duplicate specialization
    BSTNode* root2 = nullptr;
    Doctor doc2a = {"Dr. Smith", "Cardiology", 1, true};
    Doctor doc2b = {"Dr. Jones", "Cardiology", 2, false};
    root2 = insertDoctor(root2, doc2a);
    int count_before = count_doctors(root2);
    root2 = insertDoctor(root2, doc2b);
    int count_after = count_doctors(root2);
    assert(count_after == count_before + 1 && "Doctor count must increase");
    assert(root2->doctors.size() == 2 && "Same specialization in same node");
    assert(can_find_doctor(root2, doc2a) && "First doctor findable");
    assert(can_find_doctor(root2, doc2b) && "Second doctor findable");
    check_bst_property(root2);
    check_specialization_grouping(root2);
    cleanupBST(root2);
    
    // Test 3: Build BST with multiple specializations
    BSTNode* root3 = nullptr;
    Doctor doc3a = {"Dr. A", "Cardiology", 1, true};
    Doctor doc3b = {"Dr. B", "Neurology", 2, true};
    Doctor doc3c = {"Dr. C", "Anesthesia", 3, true};
    
    root3 = insertDoctor(root3, doc3a);
    root3 = insertDoctor(root3, doc3b);
    root3 = insertDoctor(root3, doc3c);
    
    assert(count_doctors(root3) == 3 && "All doctors inserted");
    assert(root3->right != nullptr && "Neurology > Cardiology (right child)");
    assert(root3->left != nullptr && "Anesthesia < Cardiology (left child)");
    check_bst_property(root3);
    check_specialization_grouping(root3);
    check_inorder_sorted(root3);
    cleanupBST(root3);
    
    // Test 4: Large dataset insertion
    BSTNode* root4 = nullptr;
    std::vector<std::string> specializations = {
        "Dermatology", "Emergency", "Gastroenterology", "Hematology",
        "Immunology", "Nephrology", "Oncology", "Pathology",
        "Psychiatry", "Radiology", "Surgery", "Urology"
    };
    
    for (size_t i = 0; i < specializations.size(); i++) {
        Doctor doc = {"Dr. Test" + std::to_string(i), specializations[i], 
                      static_cast<int>(100 + i), i % 2 == 0};
        root4 = insertDoctor(root4, doc);
        check_bst_property(root4);
    }
    
    assert(count_doctors(root4) == static_cast<int>(specializations.size()) && 
           "All doctors inserted");
    check_inorder_sorted(root4);
    check_specialization_grouping(root4);
    cleanupBST(root4);
    
    // Test 5: Stress test with duplicates
    BSTNode* root5 = nullptr;
    for (int i = 0; i < 100; i++) {
        Doctor doc = {"Dr" + std::to_string(i), "Spec" + std::to_string(i % 10), 
                      i, true};
        root5 = insertDoctor(root5, doc);
    }
    
    assert(count_doctors(root5) == 100 && "All 100 doctors inserted");
    check_bst_property(root5);
    check_specialization_grouping(root5);
    check_inorder_sorted(root5);
    cleanupBST(root5);
    
    // Test 6: Empty string specialization
    BSTNode* root6 = nullptr;
    Doctor doc6a = {"Dr. Empty", "", 1, true};
    Doctor doc6b = {"Dr. Normal", "Cardiology", 2, true};
    root6 = insertDoctor(root6, doc6a);
    root6 = insertDoctor(root6, doc6b);
    assert(count_doctors(root6) == 2 && "Both doctors inserted");
    check_bst_property(root6);
    cleanupBST(root6);
    
    // Test 7: Alphabetical ordering verification
    BSTNode* root7 = nullptr;
    std::vector<std::string> ordered = {"A", "B", "C", "D", "E"};
    for (const auto& spec : ordered) {
        Doctor doc = {"Dr. " + spec, spec, 0, true};
        root7 = insertDoctor(root7, doc);
    }
    check_inorder_sorted(root7);
    cleanupBST(root7);
    
    // Test 8: Reverse alphabetical insertion
    BSTNode* root8 = nullptr;
    std::vector<std::string> reverse = {"E", "D", "C", "B", "A"};
    for (const auto& spec : reverse) {
        Doctor doc = {"Dr. " + spec, spec, 0, true};
        root8 = insertDoctor(root8, doc);
    }
    check_bst_property(root8);
    check_inorder_sorted(root8);
    cleanupBST(root8);
    
    // Test 9: Idempotent root return
    BSTNode* root9 = nullptr;
    Doctor doc9 = {"Dr. Test", "Test", 1, true};
    BSTNode* result9 = insertDoctor(root9, doc9);
    assert(result9 != nullptr && "Must return valid root");
    root9 = result9;
    BSTNode* result9b = insertDoctor(root9, doc9);
    assert(result9b == root9 && "Root should remain same for duplicate spec");
    cleanupBST(root9);
    
    std::cout << "All insertDoctor strict tests passed!" << std::endl;
    return 0;
}
