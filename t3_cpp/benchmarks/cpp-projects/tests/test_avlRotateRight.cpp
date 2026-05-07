#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    Algorithms::avlRotateRight(root);
    assert(root->value == 1 && root->right->value == 2);
    return 0;
}
