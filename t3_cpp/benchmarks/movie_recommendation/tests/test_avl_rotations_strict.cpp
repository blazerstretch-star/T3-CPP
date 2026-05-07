#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <climits>

using namespace MovieRecommendation;

class AVLTestCleanup {
public:
    std::vector<AVLNode<int>*> nodes;
    
    AVLNode<int>* createNode(int value, int height = 0) {
        AVLNode<int>* node = new AVLNode<int>(value);
        node->height = height;
        nodes.push_back(node);
        return node;
    }
    
    ~AVLTestCleanup() {
        for (auto node : nodes) {
            delete node;
        }
    }
};

void collect_data(AVLNode<int>* node, std::vector<int>& data) {
    if (!node) return;
    data.push_back(node->data);
    collect_data(node->leftChild, data);
    collect_data(node->rightChild, data);
}

// Verify height is correctly updated
bool verify_height_updated(AVLNode<int>* node) {
    if (!node) return true;
    int leftH = avl_height(node->leftChild);
    int rightH = avl_height(node->rightChild);
    return node->height == std::max(leftH, rightH) + 1;
}

int main() {
    std::cout << "Testing avl_rotate_left..." << std::endl;
    
    // Test 1: Basic left rotation
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 2);
        AVLNode<int>* right = test.createNode(20, 1);
        AVLNode<int>* rightLeft = test.createNode(15, 0);
        
        root->rightChild = right;
        right->leftChild = rightLeft;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_left(root, treeRoot);
        
        assert(newRoot == right && "Right child becomes new root");
        assert(newRoot->leftChild == root && "Original root becomes left child");
        assert(root->rightChild == rightLeft && "Subtree moved");
        assert(verify_height_updated(newRoot) && "Height updated");
        assert(verify_height_updated(root) && "Height updated");
    }
    
    // Test 2: Rotation with NULL subtree
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 1);
        AVLNode<int>* right = test.createNode(20, 0);
        
        root->rightChild = right;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_left(root, treeRoot);
        
        assert(newRoot == right);
        assert(root->rightChild == nullptr);
        assert(verify_height_updated(newRoot));
    }
    
    // Test 3: Rotation with both children
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 2);
        AVLNode<int>* left = test.createNode(10, 0);
        AVLNode<int>* right = test.createNode(30, 1);
        AVLNode<int>* rightRight = test.createNode(40, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        right->rightChild = rightRight;
        
        AVLNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        AVLNode<int>* newRoot = avl_rotate_left(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(newRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData && "All data preserved");
        assert(root->leftChild == left && "Left child unchanged");
    }
    
    // Test 4: Root update when x is root
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 1);
        AVLNode<int>* right = test.createNode(20, 0);
        
        root->rightChild = right;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_left(root, treeRoot);
        
        assert(treeRoot == right && "Tree root updated");
        assert(newRoot == right);
    }
    
    // Test 5: Multiple rotations
    {
        AVLTestCleanup test;
        AVLNode<int>* n1 = test.createNode(10, 2);
        AVLNode<int>* n2 = test.createNode(20, 1);
        AVLNode<int>* n3 = test.createNode(30, 0);
        
        n1->rightChild = n2;
        n2->rightChild = n3;
        
        AVLNode<int>* treeRoot = n1;
        avl_rotate_left(n1, treeRoot);
        assert(verify_height_updated(treeRoot));
        
        avl_rotate_left(n1, treeRoot);
        assert(verify_height_updated(treeRoot));
    }
    
    std::cout << "✅ avl_rotate_left strict test passed (5 test cases)" << std::endl;
    
    // ========================================
    // Test avl_rotate_right
    // ========================================
    std::cout << "Testing avl_rotate_right..." << std::endl;
    
    // Test 6: Basic right rotation
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 2);
        AVLNode<int>* left = test.createNode(10, 1);
        AVLNode<int>* leftRight = test.createNode(15, 0);
        
        root->leftChild = left;
        left->rightChild = leftRight;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_right(root, treeRoot);
        
        assert(newRoot == left && "Left child becomes new root");
        assert(newRoot->rightChild == root && "Original root becomes right child");
        assert(root->leftChild == leftRight && "Subtree moved");
        assert(verify_height_updated(newRoot));
        assert(verify_height_updated(root));
    }
    
    // Test 7: Rotation with NULL subtree
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 1);
        AVLNode<int>* left = test.createNode(10, 0);
        
        root->leftChild = left;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_right(root, treeRoot);
        
        assert(newRoot == left);
        assert(root->leftChild == nullptr);
        assert(verify_height_updated(newRoot));
    }
    
    // Test 8: Rotation with both children
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(30, 2);
        AVLNode<int>* left = test.createNode(20, 1);
        AVLNode<int>* right = test.createNode(40, 0);
        AVLNode<int>* leftLeft = test.createNode(10, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = leftLeft;
        
        AVLNode<int>* treeRoot = root;
        std::vector<int> beforeData;
        collect_data(treeRoot, beforeData);
        
        AVLNode<int>* newRoot = avl_rotate_right(root, treeRoot);
        
        std::vector<int> afterData;
        collect_data(newRoot, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        assert(beforeData == afterData);
        assert(root->rightChild == right && "Right child unchanged");
    }
    
    // Test 9: Root update
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 1);
        AVLNode<int>* left = test.createNode(10, 0);
        
        root->leftChild = left;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_right(root, treeRoot);
        
        assert(treeRoot == left && "Tree root updated");
        assert(newRoot == left);
    }
    
    // Test 10: Extreme values
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(0, 1);
        AVLNode<int>* left = test.createNode(INT_MIN, 0);
        
        root->leftChild = left;
        
        AVLNode<int>* treeRoot = root;
        AVLNode<int>* newRoot = avl_rotate_right(root, treeRoot);
        
        assert(newRoot == left);
        assert(verify_height_updated(newRoot));
    }
    
    std::cout << "✅ avl_rotate_right strict test passed (5 test cases)" << std::endl;
    std::cout << "✅ All AVL rotation tests passed (10 total test cases)" << std::endl;
    
    return 0;
}
