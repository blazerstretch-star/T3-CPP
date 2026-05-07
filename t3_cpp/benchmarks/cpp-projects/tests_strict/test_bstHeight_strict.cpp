#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: height(empty) = 0
    // Property: height(single) = 1
    // Property: height(node) = 1 + max(height(left), height(right))
    
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        int height = Algorithms::bstHeight(root);
        assert(height == 0 && "Empty tree has height 0");
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        int height = Algorithms::bstHeight(root);
        assert(height == 1 && "Single node has height 1");
    }
    
    // Test 3: Two nodes - left child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        int height = Algorithms::bstHeight(root);
        assert(height == 2 && "Two nodes has height 2");
    }
    
    // Test 4: Two nodes - right child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 15);
        int height = Algorithms::bstHeight(root);
        assert(height == 2 && "Two nodes has height 2");
    }
    
    // Test 5: Balanced tree
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15, 3, 7, 12, 17};
        for (int v : values) Algorithms::bstInsert(root, v);
        int height = Algorithms::bstHeight(root);
        assert(height == 3 && "Balanced tree height");
    }
    
    // Test 6: Left-skewed tree (ascending insertion)
    {
        TreeNode* root = nullptr;
        for (int i = 5; i >= 1; i--) Algorithms::bstInsert(root, i);
        int height = Algorithms::bstHeight(root);
        assert(height == 5 && "Left-skewed tree height");
    }
    
    // Test 7: Right-skewed tree (descending insertion)
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 5; i++) Algorithms::bstInsert(root, i);
        int height = Algorithms::bstHeight(root);
        assert(height == 5 && "Right-skewed tree height");
    }
    
    // Test 8: Large balanced tree
    {
        TreeNode* root = nullptr;
        int values[] = {50, 25, 75, 12, 37, 62, 87, 6, 18, 31, 43};
        for (int v : values) Algorithms::bstInsert(root, v);
        int height = Algorithms::bstHeight(root);
        assert(height >= 3 && height <= 5 && "Large tree height in expected range");
    }
    
    return 0;
}
