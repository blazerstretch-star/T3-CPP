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
    
    std::function<bool(TreeNode*)> check_balance = [&](TreeNode* node) -> bool {
        if (!node) return true;
        int balance = Algorithms::avlGetBalance(node);
        if (balance < -1 || balance > 1) return false;
        return check_balance(node->left) && check_balance(node->right);
    };
    
    std::function<int(TreeNode*)> count_nodes = [&](TreeNode* node) -> int {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    };
    
    // Test 1: Delete from empty
    {
        TreeNode* root = nullptr;
        Algorithms::avlDelete(root, 10);
        assert(root == nullptr && "Still empty");
    }
    
    // Test 2: Delete only node
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        Algorithms::avlDelete(root, 10);
        assert(root == nullptr && "Tree empty");
    }
    
    // Test 3: Delete leaf
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        Algorithms::avlInsert(root, 5);
        Algorithms::avlInsert(root, 15);
        Algorithms::avlDelete(root, 5);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(count_nodes(root) == 2 && "One node deleted");
    }
    
    // Test 4: Delete node with one child
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        Algorithms::avlInsert(root, 5);
        Algorithms::avlInsert(root, 3);
        Algorithms::avlDelete(root, 5);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 5: Delete node with two children
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        Algorithms::avlInsert(root, 5);
        Algorithms::avlInsert(root, 15);
        Algorithms::avlInsert(root, 3);
        Algorithms::avlInsert(root, 7);
        Algorithms::avlDelete(root, 5);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 6: Delete causing rebalance
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 7; i++) {
            Algorithms::avlInsert(root, i);
        }
        Algorithms::avlDelete(root, 1);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 7: Delete multiple nodes
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 10; i++) {
            Algorithms::avlInsert(root, i);
        }
        Algorithms::avlDelete(root, 5);
        Algorithms::avlDelete(root, 3);
        Algorithms::avlDelete(root, 8);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(count_nodes(root) == 7 && "Three nodes deleted");
    }
    
    return 0;
}
