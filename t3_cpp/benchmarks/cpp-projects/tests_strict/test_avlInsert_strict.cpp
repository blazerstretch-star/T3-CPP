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
    
    // Test 1: Insert into empty
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        assert(root != nullptr && "Root created");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 2: Insert causing left-left rotation
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 30);
        Algorithms::avlInsert(root, 20);
        Algorithms::avlInsert(root, 10);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(root->value == 20 && "Root after rotation");
    }
    
    // Test 3: Insert causing right-right rotation
    {
        TreeNode* root = nullptr;
        Algorithms::avlInsert(root, 10);
        Algorithms::avlInsert(root, 20);
        Algorithms::avlInsert(root, 30);
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(root->value == 20 && "Root after rotation");
    }
    
    // Test 4: Insert multiple values
    {
        TreeNode* root = nullptr;
        int values[] = {10, 20, 30, 40, 50, 25};
        for (int v : values) {
            Algorithms::avlInsert(root, v);
        }
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(count_nodes(root) == 6 && "All nodes inserted");
    }
    
    // Test 5: Insert in ascending order
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 7; i++) {
            Algorithms::avlInsert(root, i);
        }
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 6: Insert in descending order
    {
        TreeNode* root = nullptr;
        for (int i = 7; i >= 1; i--) {
            Algorithms::avlInsert(root, i);
        }
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
    }
    
    // Test 7: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 15; i++) {
            Algorithms::avlInsert(root, i);
        }
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
        assert(check_balance(root) && "AVL balance");
        assert(count_nodes(root) == 15 && "All nodes inserted");
    }
    
    return 0;
}
