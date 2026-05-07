#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result == nullptr && "Empty tree returns nullptr");
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found node");
        assert(result->value == 10 && "Single node is max");
    }
    
    // Test 3: Max is root (no right child)
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 3);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 10 && "Root is max");
    }
    
    // Test 4: Max is right child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 15);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 15 && "Right child is max");
    }
    
    // Test 5: Max is rightmost leaf
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15, 3, 7, 12, 17, 20};
        for (int v : values) Algorithms::bstInsert(root, v);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 20 && "Rightmost leaf is max");
    }
    
    // Test 6: Ascending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 5; i++) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 5 && "Last inserted is max");
    }
    
    // Test 7: Descending insertion
    {
        TreeNode* root = nullptr;
        for (int i = 5; i >= 1; i--) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 5 && "Largest value is max");
    }
    
    // Test 8: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 50; i++) Algorithms::bstInsert(root, i);
        TreeNode* result = Algorithms::bstFindMax(root);
        assert(result != nullptr && "Found max");
        assert(result->value == 50 && "Max in large tree");
    }
    
    return 0;
}
