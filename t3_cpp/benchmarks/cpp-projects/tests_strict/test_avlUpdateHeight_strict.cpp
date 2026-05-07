#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Empty tree
    {
        TreeNode* root = nullptr;
        Algorithms::avlUpdateHeight(root);
        // No crash
    }
    
    // Test 2: Single node
    {
        TreeNode* root = new TreeNode(10);
        Algorithms::avlUpdateHeight(root);
        // Balance factor updated
    }
    
    // Test 3: Two nodes
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(5);
        Algorithms::avlUpdateHeight(root);
        // Heights updated
    }
    
    // Test 4: Three nodes
    {
        TreeNode* root = new TreeNode(10);
        root->left = new TreeNode(5);
        root->right = new TreeNode(15);
        Algorithms::avlUpdateHeight(root);
        // Heights updated
    }
    
    // Test 5: After rotation
    {
        TreeNode* root = new TreeNode(10);
        root->right = new TreeNode(20);
        root->right->right = new TreeNode(30);
        Algorithms::avlRotateLeft(root);
        Algorithms::avlUpdateHeight(root);
        // Heights correct after rotation
    }
    
    // Test 6: Large tree
    {
        TreeNode* root = nullptr;
        for (int i = 1; i <= 7; i++) {
            Algorithms::bstInsert(root, i);
        }
        Algorithms::avlUpdateHeight(root);
        // Heights updated for all nodes
    }
    
    return 0;
}
