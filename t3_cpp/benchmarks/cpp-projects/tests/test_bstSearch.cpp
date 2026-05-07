#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    TreeNode* root = nullptr;
    Algorithms::bstInsert(root, 5);
    Algorithms::bstInsert(root, 3);
    assert(Algorithms::bstSearch(root, 3) != nullptr);
    assert(Algorithms::bstSearch(root, 10) == nullptr);
    return 0;
}
