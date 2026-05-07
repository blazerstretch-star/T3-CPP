#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = nullptr;
    Algorithms::bstInsert(root, 5);
    Algorithms::bstInsert(root, 3);
    Algorithms::bstInsert(root, 7);
    assert(Algorithms::bstFindMax(root)->value == 7);
    return 0;
}
