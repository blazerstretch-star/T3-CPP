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

bool verify_parent_child_consistency(RBTNode<int>* node) {
    if (!node) return true;
    if (node->leftChild && node->leftChild->parent != node) return false;
    if (node->rightChild && node->rightChild->parent != node) return false;
    return verify_parent_child_consistency(node->leftChild) && 
           verify_parent_child_consistency(node->rightChild);
}

void collect_data(RBTNode<int>* node, std::vector<int>& data) {
    if (!node) return;
    data.push_back(node->data);
    collect_data(node->leftChild, data);
    collect_data(node->rightChild, data);
}

int main() {
    // Test 1: Basic right rotation
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(20);
        RBTNode<int>* left = test.createNode(10);
        RBTNode<int>* leftRight = test.createNode(15);
        
        root->leftChild = left;
        left->parent = root;
        left->rightChild = leftRight;
        leftRight->parent = left;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(root, treeRoot);
        
        assert(treeRoot == left && "Left child becomes new root");
        assert(left->rightChild == root && "Original root becomes right child");
        assert(root->parent == left && "Parent pointer updated");
        assert(root->leftChild == leftRight && "Subtree moved correctly");
        if (leftRight) assert(leftRight->parent == root);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 2: Rotation when x is root
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* left = test.createNode(30);
        
        root->leftChild = left;
        left->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(root, treeRoot);
        
        assert(treeRoot == left);
        assert(left->parent == nullptr && "New root has no parent");
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 3: Rotation with NULL leftRight child
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(40);
        RBTNode<int>* left = test.createNode(30);
        
        root->leftChild = left;
        left->parent = root;
        left->rightChild = nullptr;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(root, treeRoot);
        
        assert(treeRoot == left);
        assert(root->leftChild == nullptr);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 4: Rotation with both children
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(30);
        RBTNode<int>* left = test.createNode(20);
        RBTNode<int>* right = test.createNode(40);
        RBTNode<int>* leftLeft = test.createNode(10);
        RBTNode<int>* leftRight = test.createNode(25);
        
        root->leftChild = left;
        root->rightChild = right;
        left->parent = root;
        right->parent = root;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        leftLeft->parent = left;
        leftRight->parent = left;
        
        RBTNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        rb_right_rotate(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(treeRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData && "All data preserved");
        assert(verify_parent_child_consistency(treeRoot));
        assert(root->rightChild == right && "Right child unchanged");
    }
    
    // Test 5: Rotation in subtree
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* right = test.createNode(70);
        RBTNode<int>* rightLeft = test.createNode(60);
        RBTNode<int>* rightLeftLeft = test.createNode(55);
        
        root->rightChild = right;
        right->parent = root;
        right->leftChild = rightLeft;
        rightLeft->parent = right;
        rightLeft->leftChild = rightLeftLeft;
        rightLeftLeft->parent = rightLeft;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(rightLeft, treeRoot);
        
        assert(treeRoot == root && "Tree root unchanged");
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 6: Multiple consecutive rotations
    {
        RBTestCleanup test;
        RBTNode<int>* n4 = test.createNode(40);
        RBTNode<int>* n3 = test.createNode(30);
        RBTNode<int>* n2 = test.createNode(20);
        RBTNode<int>* n1 = test.createNode(10);
        
        n4->leftChild = n3;
        n3->parent = n4;
        n3->leftChild = n2;
        n2->parent = n3;
        n2->leftChild = n1;
        n1->parent = n2;
        
        RBTNode<int>* treeRoot = n4;
        
        rb_right_rotate(n4, treeRoot);
        assert(verify_parent_child_consistency(treeRoot));
        
        rb_right_rotate(n4, treeRoot);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 7: Rotation preserves colors
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(20, Colour::Black);
        RBTNode<int>* left = test.createNode(10, Colour::Red);
        
        root->leftChild = left;
        left->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(root, treeRoot);
        
        assert(root->colour == Colour::Black);
        assert(left->colour == Colour::Red);
    }
    
    // Test 8: Complex tree structure
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50);
        RBTNode<int>* left = test.createNode(30);
        RBTNode<int>* right = test.createNode(70);
        RBTNode<int>* leftLeft = test.createNode(20);
        RBTNode<int>* leftRight = test.createNode(40);
        RBTNode<int>* leftRightLeft = test.createNode(35);
        RBTNode<int>* leftRightRight = test.createNode(45);
        
        root->leftChild = left;
        root->rightChild = right;
        left->parent = root;
        right->parent = root;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        leftLeft->parent = left;
        leftRight->parent = left;
        leftRight->leftChild = leftRightLeft;
        leftRight->rightChild = leftRightRight;
        leftRightLeft->parent = leftRight;
        leftRightRight->parent = leftRight;
        
        RBTNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        rb_right_rotate(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(treeRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    // Test 9: Extreme values
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(INT_MAX);
        RBTNode<int>* left = test.createNode(INT_MIN);
        
        root->leftChild = left;
        left->parent = root;
        
        RBTNode<int>* treeRoot = root;
        rb_right_rotate(root, treeRoot);
        
        assert(treeRoot == left);
        assert(verify_parent_child_consistency(treeRoot));
    }
    
    std::cout << "✅ rb_right_rotate strict test passed (9 test cases)" << std::endl;
    return 0;
}
