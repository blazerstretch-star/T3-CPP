#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = new TreeNode(1);
    root->right = new TreeNode(2);
    Algorithms::avlRotateLeft(root);
    assert(root->value == 2 && root->left->value == 1);
    return 0;
}
