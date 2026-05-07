#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <climits>
#include <vector>
#include <cmath>

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

// PROPERTY 1: NULL nodes have height -1
bool verify_null_height() {
    return avl_height<int>(nullptr) == -1;
}

// PROPERTY 2: Function returns node's stored height
bool verify_height_matches(AVLNode<int>* node, int expectedHeight) {
    return avl_height(node) == expectedHeight;
}

// PROPERTY 3: Function is deterministic
bool verify_deterministic(AVLNode<int>* node) {
    int first = avl_height(node);
    int second = avl_height(node);
    return first == second;
}

// PROPERTY 4: Function doesn't modify node
bool verify_no_modification(AVLNode<int>* node) {
    int heightBefore = node->height;
    int dataBefore = node->data;
    AVLNode<int>* leftBefore = node->leftChild;
    AVLNode<int>* rightBefore = node->rightChild;
    
    avl_height(node);
    
    return node->height == heightBefore &&
           node->data == dataBefore &&
           node->leftChild == leftBefore &&
           node->rightChild == rightBefore;
}

int main() {
    // Test 1: NULL node returns -1
    {
        assert(avl_height<int>(nullptr) == -1 && "NULL nodes must have height -1");
        assert(verify_null_height());
    }
    
    // Test 2: Single node (height 0)
    {
        AVLTestCleanup test;
        AVLNode<int>* single = test.createNode(10, 0);
        
        assert(avl_height(single) == 0);
        assert(verify_height_matches(single, 0));
        assert(verify_deterministic(single));
        assert(verify_no_modification(single));
    }
    
    // Test 3: Various positive heights (0-100)
    {
        AVLTestCleanup test;
        for (int h = 0; h <= 100; h++) {
            AVLNode<int>* node = test.createNode(h, h);
            assert(avl_height(node) == h);
            assert(verify_deterministic(node));
        }
    }
    
    // Test 4: Negative heights (edge case - should still return stored value)
    {
        AVLTestCleanup test;
        for (int h = -10; h < 0; h++) {
            AVLNode<int>* node = test.createNode(h, h);
            assert(avl_height(node) == h && "Should return stored height even if negative");
        }
    }
    
    // Test 5: Extreme height values
    {
        AVLTestCleanup test;
        
        AVLNode<int>* maxNode = test.createNode(1, INT_MAX);
        assert(avl_height(maxNode) == INT_MAX);
        
        AVLNode<int>* minNode = test.createNode(2, INT_MIN);
        assert(avl_height(minNode) == INT_MIN);
        
        AVLNode<int>* zeroNode = test.createNode(3, 0);
        assert(avl_height(zeroNode) == 0);
    }
    
    // Test 6: Nodes in tree context
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(50, 2);
        AVLNode<int>* left = test.createNode(30, 1);
        AVLNode<int>* right = test.createNode(70, 1);
        AVLNode<int>* leftLeft = test.createNode(20, 0);
        AVLNode<int>* leftRight = test.createNode(40, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        
        assert(avl_height(root) == 2);
        assert(avl_height(left) == 1);
        assert(avl_height(right) == 1);
        assert(avl_height(leftLeft) == 0);
        assert(avl_height(leftRight) == 0);
        assert(avl_height(root->leftChild) == 1);
        assert(avl_height(root->rightChild) == 1);
    }
    
    // Test 7: Height changes are reflected
    {
        AVLTestCleanup test;
        AVLNode<int>* node = test.createNode(42, 5);
        
        assert(avl_height(node) == 5);
        
        node->height = 10;
        assert(avl_height(node) == 10);
        
        node->height = 0;
        assert(avl_height(node) == 0);
        
        node->height = -1;
        assert(avl_height(node) == -1);
    }
    
    // Test 8: Extreme data values don't affect height
    {
        AVLTestCleanup test;
        AVLNode<int>* maxData = test.createNode(INT_MAX, 5);
        AVLNode<int>* minData = test.createNode(INT_MIN, 5);
        AVLNode<int>* zeroData = test.createNode(0, 5);
        
        assert(avl_height(maxData) == 5);
        assert(avl_height(minData) == 5);
        assert(avl_height(zeroData) == 5);
    }
    
    // Test 9: Deep tree structure
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(100, 3);
        AVLNode<int>* left = test.createNode(50, 2);
        AVLNode<int>* right = test.createNode(150, 1);
        AVLNode<int>* leftLeft = test.createNode(25, 1);
        AVLNode<int>* leftRight = test.createNode(75, 0);
        AVLNode<int>* leftLeftLeft = test.createNode(10, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        leftLeft->leftChild = leftLeftLeft;
        
        assert(avl_height(root) == 3);
        assert(avl_height(left) == 2);
        assert(avl_height(right) == 1);
        assert(avl_height(leftLeft) == 1);
        assert(avl_height(leftRight) == 0);
        assert(avl_height(leftLeftLeft) == 0);
        
        // Test NULL children
        assert(avl_height(leftLeftLeft->leftChild) == -1);
        assert(avl_height(leftLeftLeft->rightChild) == -1);
        assert(avl_height(leftRight->leftChild) == -1);
        assert(avl_height(leftRight->rightChild) == -1);
    }
    
    // Test 10: Balanced tree heights
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(50, 2);
        AVLNode<int>* left = test.createNode(30, 1);
        AVLNode<int>* right = test.createNode(70, 1);
        AVLNode<int>* ll = test.createNode(20, 0);
        AVLNode<int>* lr = test.createNode(40, 0);
        AVLNode<int>* rl = test.createNode(60, 0);
        AVLNode<int>* rr = test.createNode(80, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = ll;
        left->rightChild = lr;
        right->leftChild = rl;
        right->rightChild = rr;
        
        // Verify all heights
        assert(avl_height(root) == 2);
        assert(avl_height(left) == 1);
        assert(avl_height(right) == 1);
        assert(avl_height(ll) == 0);
        assert(avl_height(lr) == 0);
        assert(avl_height(rl) == 0);
        assert(avl_height(rr) == 0);
    }
    
    // Test 11: Height consistency with max(left, right) + 1
    {
        AVLTestCleanup test;
        AVLNode<int>* root = test.createNode(50, 2);
        AVLNode<int>* left = test.createNode(30, 1);
        AVLNode<int>* right = test.createNode(70, 0);
        
        root->leftChild = left;
        root->rightChild = right;
        
        int leftHeight = avl_height(left);
        int rightHeight = avl_height(right);
        int rootHeight = avl_height(root);
        
        assert(rootHeight == std::max(leftHeight, rightHeight) + 1 &&
               "Root height should be max(left, right) + 1");
    }
    
    // Test 12: Stress test - many calls
    {
        AVLTestCleanup test;
        AVLNode<int>* node = test.createNode(42, 5);
        
        for (int i = 0; i < 10000; i++) {
            assert(avl_height(node) == 5);
            assert(avl_height<int>(nullptr) == -1);
        }
        
        assert(verify_no_modification(node));
    }
    
    // Test 13: Large tree (stress test)
    {
        AVLTestCleanup test;
        std::vector<AVLNode<int>*> nodes;
        
        // Create 1000 nodes with varying heights
        for (int i = 0; i < 1000; i++) {
            int height = i % 20;  // Heights 0-19
            AVLNode<int>* node = test.createNode(i, height);
            nodes.push_back(node);
        }
        
        // Verify all heights
        for (int i = 0; i < 1000; i++) {
            int expectedHeight = i % 20;
            assert(avl_height(nodes[i]) == expectedHeight);
        }
    }
    
    // Test 14: Height with different data types (using int template)
    {
        AVLTestCleanup test;
        AVLNode<int>* positive = test.createNode(1000, 3);
        AVLNode<int>* negative = test.createNode(-1000, 3);
        AVLNode<int>* zero = test.createNode(0, 3);
        
        assert(avl_height(positive) == 3);
        assert(avl_height(negative) == 3);
        assert(avl_height(zero) == 3);
    }
    
    // Test 15: Verify function is const-correct (doesn't modify)
    {
        AVLTestCleanup test;
        AVLNode<int>* node = test.createNode(99, 7);
        
        int heightBefore = node->height;
        int dataBefore = node->data;
        
        // Call multiple times
        for (int i = 0; i < 100; i++) {
            avl_height(node);
        }
        
        assert(node->height == heightBefore && "Height should not be modified");
        assert(node->data == dataBefore && "Data should not be modified");
    }
    
    std::cout << "✅ avl_height ultra-strict test passed (15 test cases)" << std::endl;
    std::cout << "   - NULL handling verified (-1)" << std::endl;
    std::cout << "   - Positive heights (0-100) verified" << std::endl;
    std::cout << "   - Negative heights verified" << std::endl;
    std::cout << "   - Extreme values (INT_MAX, INT_MIN) verified" << std::endl;
    std::cout << "   - Tree context verified" << std::endl;
    std::cout << "   - Height changes reflected" << std::endl;
    std::cout << "   - Function doesn't modify node" << std::endl;
    std::cout << "   - Deterministic behavior verified" << std::endl;
    std::cout << "   - Height consistency verified" << std::endl;
    std::cout << "   - Stress tested (10,000 calls + 1,000 nodes)" << std::endl;
    return 0;
}
