#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: count(empty) = 0
    // Property: count(node) = 1 + count(left) + count(right)
    
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        int count = Algorithms::bstCountNodes(root);
        assert(count == 0 && "Empty tree has 0 nodes");
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 1 && "Single node tree has 1 node");
    }
    
    // Test 3: Two nodes
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 2 && "Two node tree has 2 nodes");
    }
    
    // Test 4: Three nodes
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 15);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 3 && "Three node tree has 3 nodes");
    }
    
    // Test 5: Seven nodes (balanced)
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15, 3, 7, 12, 17};
        for (int v : values) Algorithms::bstInsert(root, v);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 7 && "Seven node tree has 7 nodes");
    }
    
    // Test 6: Ascending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 10; i++) Algorithms::bstInsert(root, i);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 10 && "Ten nodes inserted");
    }
    
    // Test 7: Descending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 10; i >= 1; i--) Algorithms::bstInsert(root, i);
        int count = Algorithms::bstCountNodes(root);
        assert(count == 10 && "Ten nodes inserted");
    }
    
    // Test 8: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 0; i < 100; i++) {
            Algorithms::bstInsert(root, i * 2);
        }
        int count = Algorithms::bstCountNodes(root);
        assert(count == 100 && "100 nodes inserted");
    }
    
    return 0;
}
