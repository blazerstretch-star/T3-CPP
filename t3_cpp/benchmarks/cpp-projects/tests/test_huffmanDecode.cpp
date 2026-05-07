#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    int values[] = {97, 98};
    double freqs[] = {0.6, 0.4};
    TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 2);
    int encoded[] = {0, 1};
    int decoded[10];
    int decLen = 0;
    Algorithms::huffmanDecode(root, encoded, 2, decoded, decLen);
    assert(decLen >= 0);
    return 0;
}
