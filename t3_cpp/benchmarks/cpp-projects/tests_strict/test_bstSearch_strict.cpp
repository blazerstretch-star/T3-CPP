#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Search in empty tree
    {
        TreeNode* root = nullptr;
        TreeNode* result = Algorithms::bstSearch(root, 5);
        assert(result == nullptr && "Search in empty tree returns nullptr");
    }
    
    // Test 2: Search for root
    {
        TreeNode* root = new TreeNode(10);
        TreeNode* result = Algorithms::bstSearch(root, 10);
        assert(result != nullptr && "Found root");
        assert(result->value == 10 && "Correct value");
    }
    
    // Test 3: Search for existing left child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        TreeNode* result = Algorithms::bstSearch(root, 5);
        assert(result != nullptr && "Found left child");
        assert(result->value == 5 && "Correct value");
    }
    
    // Test 4: Search for existing right child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 15);
        TreeNode* result = Algorithms::bstSearch(root, 15);
        assert(result != nullptr && "Found right child");
        assert(result->value == 15 && "Correct value");
    }
    
    // Test 5: Search for non-existing element
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 15);
        TreeNode* result = Algorithms::bstSearch(root, 20);
        assert(result == nullptr && "Non-existing element not found");
    }
    
    // Test 6: Search in larger tree
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15, 3, 7, 12, 17};
        for (int v : values) Algorithms::bstInsert(root, v);
        
        for (int v : values) {
            TreeNode* result = Algorithms::bstSearch(root, v);
            assert(result != nullptr && "All inserted values found");
            assert(result->value == v && "Correct value found");
        }
    }
    
    // Test 7: Search for values not in tree
    {
        TreeNode* root = nullptr;
        int values[] = {10, 5, 15};
        for (int v : values) Algorithms::bstInsert(root, v);
        
        assert(Algorithms::bstSearch(root, 1) == nullptr && "1 not found");
        assert(Algorithms::bstSearch(root, 7) == nullptr && "7 not found");
        assert(Algorithms::bstSearch(root, 20) == nullptr && "20 not found");
    }
    
    // Test 8: Deterministic
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 10);
        Algorithms::bstInsert(root, 5);
        TreeNode* result1 = Algorithms::bstSearch(root, 5);
        TreeNode* result2 = Algorithms::bstSearch(root, 5);
        assert(result1 == result2 && "Search is deterministic");
    }
    
    return 0;
}
