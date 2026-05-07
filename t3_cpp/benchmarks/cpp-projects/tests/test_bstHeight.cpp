#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = nullptr;
    Algorithms::bstInsert(root, 5);
    Algorithms::bstInsert(root, 3);
    Algorithms::bstInsert(root, 7);
    assert(Algorithms::bstHeight(root) == 2);
    return 0;
}
