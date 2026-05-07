#include "../src/movie_functions.h"
#include <cassert>
#include <iostream>
#include <vector>
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

// PROPERTY 1: NULL nodes are always black
bool verify_null_is_black() {
    return rb_get_colour<int>(nullptr) == Colour::Black;
}

// PROPERTY 2: Function returns node's actual color
bool verify_color_matches(RBTNode<int>* node, Colour expectedColor) {
    return rb_get_colour(node) == expectedColor;
}

// PROPERTY 3: Function is deterministic (same input → same output)
bool verify_deterministic(RBTNode<int>* node) {
    Colour first = rb_get_colour(node);
    Colour second = rb_get_colour(node);
    return first == second;
}

int main() {
    // Test 1: NULL node returns Black
    {
        assert(rb_get_colour<int>(nullptr) == Colour::Black && 
               "NULL nodes must be considered Black");
        assert(verify_null_is_black());
    }
    
    // Test 2: Red node returns Red
    {
        RBTestCleanup test;
        RBTNode<int>* redNode = test.createNode(10, Colour::Red);
        
        assert(rb_get_colour(redNode) == Colour::Red);
        assert(verify_color_matches(redNode, Colour::Red));
        assert(verify_deterministic(redNode));
    }
    
    // Test 3: Black node returns Black
    {
        RBTestCleanup test;
        RBTNode<int>* blackNode = test.createNode(20, Colour::Black);
        
        assert(rb_get_colour(blackNode) == Colour::Black);
        assert(verify_color_matches(blackNode, Colour::Black));
        assert(verify_deterministic(blackNode));
    }
    
    // Test 4: Multiple red nodes
    {
        RBTestCleanup test;
        for (int i = 0; i < 100; i++) {
            RBTNode<int>* node = test.createNode(i, Colour::Red);
            assert(rb_get_colour(node) == Colour::Red);
        }
    }
    
    // Test 5: Multiple black nodes
    {
        RBTestCleanup test;
        for (int i = 0; i < 100; i++) {
            RBTNode<int>* node = test.createNode(i, Colour::Black);
            assert(rb_get_colour(node) == Colour::Black);
        }
    }
    
    // Test 6: Nodes in tree context
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(50, Colour::Black);
        RBTNode<int>* left = test.createNode(30, Colour::Red);
        RBTNode<int>* right = test.createNode(70, Colour::Black);
        
        root->leftChild = left;
        root->rightChild = right;
        left->parent = root;
        right->parent = root;
        
        assert(rb_get_colour(root) == Colour::Black);
        assert(rb_get_colour(left) == Colour::Red);
        assert(rb_get_colour(right) == Colour::Black);
        assert(rb_get_colour(root->leftChild) == Colour::Red);
        assert(rb_get_colour(root->rightChild) == Colour::Black);
    }
    
    // Test 7: Color changes are reflected
    {
        RBTestCleanup test;
        RBTNode<int>* node = test.createNode(42, Colour::Red);
        
        assert(rb_get_colour(node) == Colour::Red);
        
        node->colour = Colour::Black;
        assert(rb_get_colour(node) == Colour::Black);
        
        node->colour = Colour::Red;
        assert(rb_get_colour(node) == Colour::Red);
    }
    
    // Test 8: Extreme data values don't affect color
    {
        RBTestCleanup test;
        RBTNode<int>* maxRed = test.createNode(INT_MAX, Colour::Red);
        RBTNode<int>* minRed = test.createNode(INT_MIN, Colour::Red);
        RBTNode<int>* maxBlack = test.createNode(INT_MAX, Colour::Black);
        RBTNode<int>* minBlack = test.createNode(INT_MIN, Colour::Black);
        
        assert(rb_get_colour(maxRed) == Colour::Red);
        assert(rb_get_colour(minRed) == Colour::Red);
        assert(rb_get_colour(maxBlack) == Colour::Black);
        assert(rb_get_colour(minBlack) == Colour::Black);
    }
    
    // Test 9: Deep tree structure
    {
        RBTestCleanup test;
        RBTNode<int>* root = test.createNode(100, Colour::Black);
        RBTNode<int>* left = test.createNode(50, Colour::Red);
        RBTNode<int>* right = test.createNode(150, Colour::Red);
        RBTNode<int>* leftLeft = test.createNode(25, Colour::Black);
        RBTNode<int>* leftRight = test.createNode(75, Colour::Black);
        RBTNode<int>* rightLeft = test.createNode(125, Colour::Black);
        RBTNode<int>* rightRight = test.createNode(175, Colour::Black);
        
        root->leftChild = left;
        root->rightChild = right;
        left->leftChild = leftLeft;
        left->rightChild = leftRight;
        right->leftChild = rightLeft;
        right->rightChild = rightRight;
        
        assert(rb_get_colour(root) == Colour::Black);
        assert(rb_get_colour(left) == Colour::Red);
        assert(rb_get_colour(right) == Colour::Red);
        assert(rb_get_colour(leftLeft) == Colour::Black);
        assert(rb_get_colour(leftRight) == Colour::Black);
        assert(rb_get_colour(rightLeft) == Colour::Black);
        assert(rb_get_colour(rightRight) == Colour::Black);
        
        // Test NULL children
        assert(rb_get_colour(leftLeft->leftChild) == Colour::Black);
        assert(rb_get_colour(leftLeft->rightChild) == Colour::Black);
    }
    
    // Test 10: Alternating colors
    {
        RBTestCleanup test;
        std::vector<RBTNode<int>*> nodes;
        
        for (int i = 0; i < 50; i++) {
            Colour color = (i % 2 == 0) ? Colour::Black : Colour::Red;
            RBTNode<int>* node = test.createNode(i, color);
            nodes.push_back(node);
        }
        
        for (int i = 0; i < 50; i++) {
            Colour expected = (i % 2 == 0) ? Colour::Black : Colour::Red;
            assert(rb_get_colour(nodes[i]) == expected);
        }
    }
    
    // Test 11: Function doesn't modify node
    {
        RBTestCleanup test;
        RBTNode<int>* node = test.createNode(99, Colour::Red);
        
        Colour before = node->colour;
        int dataBefore = node->data;
        RBTNode<int>* leftBefore = node->leftChild;
        RBTNode<int>* rightBefore = node->rightChild;
        RBTNode<int>* parentBefore = node->parent;
        
        rb_get_colour(node);
        
        assert(node->colour == before && "Color should not change");
        assert(node->data == dataBefore && "Data should not change");
        assert(node->leftChild == leftBefore && "Left child should not change");
        assert(node->rightChild == rightBefore && "Right child should not change");
        assert(node->parent == parentBefore && "Parent should not change");
    }
    
    // Test 12: Stress test - many calls
    {
        RBTestCleanup test;
        RBTNode<int>* redNode = test.createNode(1, Colour::Red);
        RBTNode<int>* blackNode = test.createNode(2, Colour::Black);
        
        for (int i = 0; i < 10000; i++) {
            assert(rb_get_colour(redNode) == Colour::Red);
            assert(rb_get_colour(blackNode) == Colour::Black);
            assert(rb_get_colour<int>(nullptr) == Colour::Black);
        }
    }
    
    std::cout << "✅ rb_get_colour ultra-strict test passed (12 test cases)" << std::endl;
    std::cout << "   - NULL handling verified" << std::endl;
    std::cout << "   - Red/Black colors verified" << std::endl;
    std::cout << "   - Deterministic behavior verified" << std::endl;
    std::cout << "   - Tree context verified" << std::endl;
    std::cout << "   - Color changes reflected" << std::endl;
    std::cout << "   - Function doesn't modify node" << std::endl;
    std::cout << "   - Stress tested (10,000 calls)" << std::endl;
    return 0;
}
