#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = nullptr;
    Algorithms::avlInsert(root, 10);
    Algorithms::avlInsert(root, 20);
    Algorithms::avlDelete(root, 10);
    assert(root->value == 20);
    return 0;
}
