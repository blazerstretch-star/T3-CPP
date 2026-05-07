#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    root->right = new TreeNode(3);
    assert(Algorithms::avlGetBalance(root) == 0);
    return 0;
}
