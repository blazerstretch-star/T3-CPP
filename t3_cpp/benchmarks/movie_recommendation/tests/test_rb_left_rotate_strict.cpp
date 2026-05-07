#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace MovieRecommendation;

class RBTestCleanup {
public:
    std::vector<RBTNode<int>*> nodes;
    
    RBTNode<int>* createNode(int value, Colour color = Colour::Red) {
        RBTNode<int>* node = new RBTNode<int>(value, color);
        nodes.push_back(node);
        return node;
    }
    
    ~RBTestCleanup() {
        for (auto node : nodes) {
            delete node;
        }
    }
};

// PROPERTY 1: Parent-child consistency
bool verify_parent_child_consistency(RBTNode<int>* node) {
    if (!node) return true;
    
    if (node->leftChild) {
        if (node->leftChild->parent != node) return false;
        if (!verify_parent_child_consistency(node->leftChild)) return false;
    }
    
    if (node->rightChild) {
        if (node->rightChild->parent != node) return false;
        if (!verify_parent_child_consistency(node->rightChild)) return false;
    }
    
    return true;
}

// PROPERTY 2: Data preservation
void collect_data(RBTNode<int>* node, std::vector<int>& data) {
    if (!node) return;
    data.push_back(node->data);
    collect_data(node->leftChild, data);
    collect_data(node->rightChild, data);
}

// PROPERTY 3: BST property maintained
bool verify_bst_property(RBTNode<int>* node, int minVal, int maxVal) {
    if (!node) return true;
    if (node->data < minVal || node->data > maxVal) return false;
    return verify_bst_property(node->leftChild, minVal, node->data - 1) &&
           verify_bst_property(node->rightChild, node->data + 1, maxVal);
}

int main() {
    // Test 1: Basic left rotation
    {
        std::cout << "Test 1: Basic left rotation" << std::endl;
        RBTestCleanup test;
        std::cout << "Creating root node..." << std::endl;
        RBTNode<int>* root = test.createNode(10);
        std::cout << "Creating right node..." << std::endl;
        RBTNode<int>* right = test.createNode(20);
        std::cout << "Creating rightLeft node..." << std::endl;
        RBTNode<int>* rightLeft = test.createNode(15);
        
        std::cout << "Setting up tree structure..." << std::endl;
        root->rightChild = right;
        right->parent = root;
        right->leftChild = rightLeft;
        rightLeft->parent = right;
        
        std::cout << "Calling rb_left_rotate..." << std::endl;
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(root, treeRoot);
        
        // After rotation: right becomes new root
        assert(treeRoot == right && "Right child should become new root");
        assert(right->leftChild == root && "Original root becomes left child");
        assert(root->parent == right && "Parent pointer updated");
        assert(root->rightChild == rightLeft && "Subtree moved correctly");
        if (rightLeft) assert(rightLeft->parent == root && "Subtree parent updated");
        
        assert(verify_parent_child_consistency(treeRoot) && "Parent-child consistency");
    }
    
    // Test 2: Rotation when x is root
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* right = test.createNode(70);
        
        root->rightChild = right;
        right->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(root, treeRoot);
        
        assert(treeRoot == right && "New root should be right child");
        assert(right->parent == nullptr && "New root has no parent");
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 3: Rotation with NULL rightLeft child
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(30);
        RBTNode<int>* right = test.createNode(40);
        
        root->rightChild = right;
        right->parent = root;
        right->leftChild = nullptr; // No left child
        
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(root, treeRoot);
        
        assert(treeRoot == right);
        assert(root->rightChild == nullptr && "Should be NULL after rotation");
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 4: Rotation with both children
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(20);
        RBTNode<int>* left = test.createNode(10);
        RBTNode<int>* right = test.createNode(30);
        RBTNode<int>* rightLeft = test.createNode(25);
        RBTNode<int>* rightRight = test.createNode(40);
        
        root->leftChild = left;
        root->rightChild = right;
        left->parent = root;
        right->parent = root;
        right->leftChild = rightLeft;
        right->rightChild = rightRight;
        rightLeft->parent = right;
        rightRight->parent = right;
        
        RBTNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        rb_left_rotate(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(treeRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData && "All data preserved");
        assert(verify_parent_child_consistency(treeRoot));
        assert(treeRoot == right);
        assert(root->leftChild == left && "Left child unchanged");
    }
    
    // Test 5: Rotation in subtree (x is not root)
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* left = test.createNode(30);
        RBTNode<int>* leftRight = test.createNode(40);
        RBTNode<int>* leftRightRight = test.createNode(45);
        
        root->leftChild = left;
        left->parent = root;
        left->rightChild = leftRight;
        leftRight->parent = left;
        leftRight->rightChild = leftRightRight;
        leftRightRight->parent = leftRight;
        
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(leftRight, treeRoot);
        
        // Root should not change
        assert(treeRoot == root && "Tree root unchanged");
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 6: Multiple consecutive rotations
    {
        RBTestCleanup test;
        RBTNode<int>* n1 = test.createNode(10);
        RBTNode<int>* n2 = test.createNode(20);
        RBTNode<int>* n3 = test.createNode(30);
        RBTNode<int>* n4 = test.createNode(40);
        
        n1->rightChild = n2;
        n2->parent = n1;
        n2->rightChild = n3;
        n3->parent = n2;
        n3->rightChild = n4;
        n4->parent = n3;
        
        RBTNode<int>* treeRoot = n1;
        
        rb_left_rotate(n1, treeRoot);
        assert(verify_parent_child_consistency(treeRoot));
        
        rb_left_rotate(n1, treeRoot);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 7: Rotation preserves colors
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(10, Colour::Black);
        RBTNode<int>* right = test.createNode(20, Colour::Red);
        
        root->rightChild = right;
        right->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(root, treeRoot);
        
        assert(root->colour == Colour::Black && "Colors preserved");
        assert(right->colour == Colour::Red && "Colors preserved");
    }
    
    // Test 8: Complex tree structure
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* left = test.createNode(30);
        RBTNode<int>* right = test.createNode(70);
        RBTNode<int>* rightLeft = test.createNode(60);
        RBTNode<int>* rightRight = test.createNode(80);
        RBTNode<int>* rightLeftLeft = test.createNode(55);
        RBTNode<int>* rightLeftRight = test.createNode(65);
        
        root->leftChild = left;
        root->rightChild = right;
        left->parent = root;
        right->parent = root;
        right->leftChild = rightLeft;
        right->rightChild = rightRight;
        rightLeft->parent = right;
        rightRight->parent = right;
        rightLeft->leftChild = rightLeftLeft;
        rightLeft->rightChild = rightLeftRight;
        rightLeftLeft->parent = rightLeft;
        rightLeftRight->parent = rightLeft;
        
        RBTNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        rb_left_rotate(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(treeRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData && "All 7 nodes preserved");
        assert(verify_parent_child_consistency(treeRoot));
        assert(treeRoot == right);
    }
    
    // Test 9: Extreme values
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(INT_MIN);
        RBTNode<int>* right = test.createNode(INT_MAX);
        
        root->rightChild = right;
        right->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_left_rotate(root, treeRoot);
        
        assert(treeRoot == right);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    std::cout << "✅ rb_left_rotate strict test passed (9 test cases)" << std::endl;
    return 0;
}
