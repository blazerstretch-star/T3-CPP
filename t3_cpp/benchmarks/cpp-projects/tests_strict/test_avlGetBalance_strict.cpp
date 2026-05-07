#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Property: balance = height(left) - height(right)
    // Property: |balance| <= 1 for AVL tree
    
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == 0 && "Empty tree balance is 0");
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == 0 && "Single node balance is 0");
    }
    
    // Test 3: Left-heavy
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(5);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == 1 && "Left-heavy balance is 1");
    }
    
    // Test 4: Right-heavy
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(15);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == -1 && "Right-heavy balance is -1");
    }
    
    // Test 5: Balanced
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(5);
        root->right = new TreeNode(15);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == 0 && "Balanced tree balance is 0");
    }
    
    // Test 6: Left-left case
    {
        TreeNode* root = new TreeNode(30);
        root->left = new TreeNode(20);
        root->left->left = new TreeNode(10);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == 2 && "Left-left balance is 2");
    }
    
    // Test 7: Right-right case
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(20);
        root->right->right = new TreeNode(30);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance == -2 && "Right-right balance is -2");
    }
    
    // Test 8: Complex tree
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(5);
        root->right = new TreeNode(15);
        root->left->left = new TreeNode(3);
        root->left->right = new TreeNode(7);
        int balance = Algorithms::avlGetBalance(root);
        assert(balance >= -1 && balance <= 1 && "Balanced tree");
    }
    
    return 0;
}
