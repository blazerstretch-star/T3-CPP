#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <climits>
#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>

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

// PROPERTY 1: AVL balance property - balance factor must be in [-1, 0, 1]
bool verify_avl_balance_property(AVLNode<int>* node) {
    if (!node) return true;
    
    int leftHeight = avl_height(node->leftChild);
    int rightHeight = avl_height(node->rightChild);
    int balance = leftHeight - rightHeight;
    
    if (abs(balance) > 1) return false;
    
    return verify_avl_balance_property(node->leftChild) && 
           verify_avl_balance_property(node->rightChild);
}

// PROPERTY 2: Height correctness - node height = max(left, right) + 1
bool verify_height_correctness(AVLNode<int>* node) {
    if (!node) return true;
    
    int leftHeight = avl_height(node->leftChild);
    int rightHeight = avl_height(node->rightChild);
    int expectedHeight = std::max(leftHeight, rightHeight) + 1;
    
    if (node->height != expectedHeight) return false;
    
    return verify_height_correctness(node->leftChild) && 
           verify_height_correctness(node->rightChild);
}

// PROPERTY 3: Data preservation
void collect_data(AVLNode<int>* node, std::vector<int>& data) {
    if (!node) return;
    data.push_back(node->data);
    collect_data(node->leftChild, data);
    collect_data(node->rightChild, data);
}

int main() {
    // Test 1: Balanced single node (no rebalancing needed)
    {
        AVLTestCleanup test;
        AVLNode<int>* node = test.createNode(10, 0);
        AVLNode<int>* result = avl_rebalance(node);
        
        assert(result != nullptr && "Result should not be NULL");
        assert(verify_avl_balance_property(result) && "Balance property must hold");
        assert(verify_height_correctness(result) && "Height must be correct");
    }
    
    // Test 2: Left-heavy tree (balance factor = 2)
    // Note: Implementation may use any valid rebalancing strategy
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(30, 2);
        AVLNode<int>* left = test.createNode(20, 1);
        AVLNode<int>* leftLeft = test.createNode(10, 0);
        root->leftChild = left;
        left->leftChild = leftLeft;
        
        std::vector<int> beforeData;
        collect_data(root, beforeData);
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        std::vector<int> afterData;
        collect_data(balanced, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        // ✅ REQUIRED: Properties must hold (implementation-agnostic)
        assert(balanced != nullptr && "Result should not be NULL");
        assert(verify_avl_balance_property(balanced) && "Balance property must hold after rebalancing");
        assert(verify_height_correctness(balanced) && "Heights must be recalculated correctly");
        assert(beforeData == afterData && "All data must be preserved");
        
        // ℹ️ NOTE: We don't check HOW it was rebalanced (rotation type is implementation detail)
    }
    
    // Test 3: Right-heavy tree (balance factor = -2)
    // Note: Implementation may use any valid rebalancing strategy
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 2);
        AVLNode<int>* right = test.createNode(20, 1);
        AVLNode<int>* rightRight = test.createNode(30, 0);
        root->rightChild = right;
        right->rightChild = rightRight;
        
        std::vector<int> beforeData;
        collect_data(root, beforeData);
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        std::vector<int> afterData;
        collect_data(balanced, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        // ✅ REQUIRED: Properties must hold
        assert(balanced != nullptr);
        assert(verify_avl_balance_property(balanced));
        assert(verify_height_correctness(balanced));
        assert(beforeData == afterData);
    }
    
    // Test 4: Left-Right imbalance (balance factor = 2, left subtree right-heavy)
    // Note: Implementation may fix this using any valid strategy
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(30, 2);
        AVLNode<int>* left = test.createNode(10, 1);
        AVLNode<int>* leftRight = test.createNode(20, 0);
        root->leftChild = left;
        left->rightChild = leftRight;
        
        std::vector<int> beforeData;
        collect_data(root, beforeData);
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        std::vector<int> afterData;
        collect_data(balanced, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        // ✅ REQUIRED: Properties must hold
        assert(balanced != nullptr);
        assert(verify_avl_balance_property(balanced));
        assert(verify_height_correctness(balanced));
        assert(beforeData == afterData);
    }
    
    // Test 5: Right-Left imbalance (balance factor = -2, right subtree left-heavy)
    // Note: Implementation may fix this using any valid strategy
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 2);
        AVLNode<int>* right = test.createNode(30, 1);
        AVLNode<int>* rightLeft = test.createNode(20, 0);
        root->rightChild = right;
        right->leftChild = rightLeft;
        
        std::vector<int> beforeData;
        collect_data(root, beforeData);
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        std::vector<int> afterData;
        collect_data(balanced, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        // ✅ REQUIRED: Properties must hold
        assert(balanced != nullptr);
        assert(verify_avl_balance_property(balanced));
        assert(verify_height_correctness(balanced));
        assert(beforeData == afterData);
    }
    
    // Test 6: Already balanced tree (balance factor = 1)
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 1);
        AVLNode<int>* left = test.createNode(10, 0);
        root->leftChild = left;
        
        AVLNode<int>* result = avl_rebalance(root);
        
        // ✅ REQUIRED: Properties must hold (even if no rebalancing needed)
        assert(result != nullptr);
        assert(verify_avl_balance_property(result));
        assert(verify_height_correctness(result));
    }
    
    // Test 7: Already balanced tree (balance factor = -1)
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(10, 1);
        AVLNode<int>* right = test.createNode(20, 0);
        root->rightChild = right;
        
        AVLNode<int>* result = avl_rebalance(root);
        
        // ✅ REQUIRED: Properties must hold
        assert(result != nullptr);
        assert(verify_avl_balance_property(result));
        assert(verify_height_correctness(result));
    }
    
    // Test 8: Perfectly balanced tree (balance factor = 0)
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(20, 1);
        AVLNode<int>* left = test.createNode(10, 0);
        AVLNode<int>* right = test.createNode(30, 0);
        root->leftChild = left;
        root->rightChild = right;
        
        AVLNode<int>* result = avl_rebalance(root);
        
        // ✅ REQUIRED: Properties must hold
        assert(result != nullptr);
        assert(verify_avl_balance_property(result));
        assert(verify_height_correctness(result));
        assert(result->height == 1 && "Root height should be 1");
    }
    
    // Test 9: Extreme values
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(0, 2);
        AVLNode<int>* left = test.createNode(INT_MIN, 1);
        AVLNode<int>* leftLeft = test.createNode(INT_MIN + 1, 0);
        root->leftChild = left;
        left->leftChild = leftLeft;
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        // ✅ REQUIRED: Properties must hold with extreme values
        assert(balanced != nullptr);
        assert(verify_avl_balance_property(balanced));
        assert(verify_height_correctness(balanced));
    }
    
    // Test 10: Larger imbalanced tree
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(50, 3);
        AVLNode<int>* left = test.createNode(30, 2);
        AVLNode<int>* right = test.createNode(60, 0);
        AVLNode<int>* leftLeft = test.createNode(20, 1);
        AVLNode<int>* leftRight = test.createNode(40, 0);
        AVLNode<int>* leftLeftLeft = test.createNode(10, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        leftLeft->leftChild = leftLeftLeft;
        
        std::vector<int> beforeData;
        collect_data(root, beforeData);
        
        AVLNode<int>* balanced = avl_rebalance(root);
        
        std::vector<int> afterData;
        collect_data(balanced, afterData);
        std::sort(beforeData.begin(), beforeData.end());
        std::sort(afterData.begin(), afterData.end());
        
        // ✅ REQUIRED: Properties must hold
        assert(balanced != nullptr);
        assert(verify_avl_balance_property(balanced));
        assert(verify_height_correctness(balanced));
        assert(beforeData == afterData);
    }
    
    std::cout << "✅ avl_rebalance fully generalized test passed (10 test cases)" << std::endl;
    std::cout << "   ✅ Balance property verified (REQUIRED)" << std::endl;
    std::cout << "   ✅ Height correctness verified (REQUIRED)" << std::endl;
    std::cout << "   ✅ Data preservation verified (REQUIRED)" << std::endl;
    std::cout << "   ℹ️  No assumptions about rotation types" << std::endl;
    std::cout << "   ℹ️  Implementation-agnostic (any valid rebalancing accepted)" << std::endl;
    std::cout << "   ℹ️  Tests verify WHAT (properties) not HOW (algorithm)" << std::endl;
    return 0;
}
