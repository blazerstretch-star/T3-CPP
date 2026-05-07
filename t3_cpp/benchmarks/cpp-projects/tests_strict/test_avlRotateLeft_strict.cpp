#include "../src/ds_algorithms.h"
#include <cassert>
#include <vector>
#include <climits>
#include <functional>

int main() {
    std::function<bool(TreeNode*, int, int)> validate_bst = [&](TreeNode* node, int min_val, int max_val) -> bool {
        if (!node) return true;
        if (node->value <= min_val || node->value >= max_val) return false;
        return validate_bst(node->left, min_val, node->value) &&
               validate_bst(node->right, node->value, max_val);
    };
    
    std::function<void(TreeNode*, std::vector<int>&)> inorder = [&](TreeNode* node, std::vector<int>& result) {
        if (!node) return;
        inorder(node->left, result);
        result.push_back(node->value);
        inorder(node->right, result);
    };
    
    // Test 1: Simple left rotation
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(20);
        root->right->right = new TreeNode(30);
        std::vector<int> before;
        inorder(root, before);
        Algorithms::avlRotateLeft(root);
        std::vector<int> after;
        inorder(root, after);
        assert(before == after && "Inorder unchanged");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
    }
    
    // Test 2: Rotation with left child
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(20);
        root->right->left = new TreeNode(15);
        root->right->right = new TreeNode(30);
        std::vector<int> before;
        inorder(root, before);
        Algorithms::avlRotateLeft(root);
        std::vector<int> after;
        inorder(root, after);
        assert(before == after && "Inorder unchanged");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
    }
    
    // Test 3: Multiple rotations
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(20);
        root->right->right = new TreeNode(30);
        Algorithms::avlRotateLeft(root);
        assert(root->value == 20 && "New root");
        assert(validate_bst(root, INT_MIN, INT_MAX) && "BST property");
    }
    
    return 0;
}
