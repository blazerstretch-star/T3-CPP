#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int values[] = {97, 98, 99};
    double freqs[] = {0.5, 0.3, 0.2};
    TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 3);
    int path[100], codes[10][100], codeCount = 0;
    Algorithms::huffmanGenerateCodes(root, path, 0, codes, codeCount);
    assert(codeCount > 0);
    return 0;
}
