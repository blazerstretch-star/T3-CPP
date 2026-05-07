#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result == nullptr && "Empty tree returns nullptr");
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found node");
        assert(result->value == 10 && "Single node is min");
    }
    
    // Test 3: Min is root (no left child)
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 15);
        Algorithms::bstInsert(root, 20);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 10 && "Root is min");
    }
    
    // Test 4: Min is left child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 15);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 5 && "Left child is min");
    }
    
    // Test 5: Min is leftmost leaf
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15, 3, 7, 12, 17, 1};
        for (int v : values) Algorithms::bstInsert(root, v);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 1 && "Leftmost leaf is min");
    }
    
    // Test 6: Ascending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 5; i++) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 1 && "First inserted is min");
    }
    
    // Test 7: Descending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 5; i >= 1; i--) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 1 && "Smallest value is min");
    }
    
    // Test 8: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 50; i >= 1; i--) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMin(root);
        assert(result != nullptr && "Found min");
        assert(result->value == 1 && "Min in large tree");
    }
    
    return 0;
}
