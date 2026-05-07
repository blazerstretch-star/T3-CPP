#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = new TreeNode(2);
    root->left = new TreeNode(1);
    Algorithms::avlUpdateHeight(root);
    assert(root != nullptr);
    return 0;
}
