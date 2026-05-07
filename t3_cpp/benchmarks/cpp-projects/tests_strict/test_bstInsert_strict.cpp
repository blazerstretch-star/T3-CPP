#include "../src/ds_algorithms.h"
#include <cassert>
#include <climits>
#include <functional>

int main() {
    std::function<bool(TreeNode*, int, int)> validate_bst = [&](TreeNode* node, int min_val, int max_val) -> bool {
        if (!node) return true;
        if (node->value <= min_val || node->value >= max_val) return false;
        return validate_bst(node->left, min_val, node->value) &&
               validate_bst(node->right, node->value, max_val);
    };
    
    std::function<int(TreeNode*)> count_nodes = [&](TreeNode* node) -> int {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    };
    
    // Test 1: Insert into empty tree
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 5);
        assert(root != nullptr && "Root created");
        assert(root->value == 5 && "Root has correct value");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 1 && "One node");
    }
    
    // Test 2: Insert left child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 3);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 2 && "Two nodes");
        assert(root->left != nullptr && root->left->value == 3 && "Left child correct");
    }
    
    // Test 3: Insert right child
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 7);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 2 && "Two nodes");
        assert(root->right != nullptr && root->right->value == 7 && "Right child correct");
    }
    
    // Test 4: Insert multiple nodes
    {
        TreeNode* root = nullptr;
        int values[] = {5, 3, 7, 1, 4, 6, 9};
        for (int v : values) Algorithms::bstInsert(root, v);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 7 && "All nodes inserted");
    }
    
    // Test 5: Insert in ascending order
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 5; i++) Algorithms::bstInsert(root, i);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 5 && "All nodes inserted");
    }
    
    // Test 6: Insert in descending order
    {
        TreeNode* root = nullptr;
        for (int i = 5; i >= 1; i--) Algorithms::bstInsert(root, i);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 5 && "All nodes inserted");
    }
    
    // Test 7: Insert duplicate
    {
        TreeNode* root = nullptr;
        Algorithms::bstInsert(root, 5);
        Algorithms::bstInsert(root, 5);
        int node_count = count_nodes(root);
        assert((node_count == 1 || node_count == 2) && "Duplicate handling");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
    }
    
    // Test 8: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 0; i < 100; i++) {
            Algorithms::bstInsert(root, i * 2);
        }
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property holds");
        assert(count_nodes(root) == 100 && "All nodes inserted");
    }
    
    return 0;
}
