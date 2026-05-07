#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    auto is_prefix_free = [](int codes[][100], int count) {
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < count; j++) {
                if (i == j) continue;
                bool is_prefix = true;
                int k = 0;
                while (codes[i][k] != -1 && codes[j][k] != -1) {
                    if (codes[i][k] != codes[j][k]) {
                        is_prefix = false;
                        break;
                    }
                    k++;
                }
                if (is_prefix && codes[i][k] == -1 && codes[j][k] != -1) {
                    return false;
                }
            }
        }
        return true;
    };
    
    // Test 1: Simple tree
    {
        int values[] = {97, 98};
        double freqs[] = {0.6, 0.4};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 2);
        int path[100];
        int codes[10][100];
        int codeCount = 0;
        Algorithms::huffmanGenerateCodes(root, path, 0, codes, codeCount);
        assert(codeCount == 2 && "Two codes generated");
        assert(is_prefix_free(codes, codeCount) && "Prefix-free property");
    }
    
    // Test 2: Three values
    {
        int values[] = {97, 98, 99};
        double freqs[] = {0.5, 0.3, 0.2};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 3);
        int path[100];
        int codes[10][100];
        int codeCount = 0;
        Algorithms::huffmanGenerateCodes(root, path, 0, codes, codeCount);
        assert(codeCount == 3 && "Three codes generated");
        assert(is_prefix_free(codes, codeCount) && "Prefix-free property");
    }
    
    // Test 3: Four values
    {
        int values[] = {1, 2, 3, 4};
        double freqs[] = {0.25, 0.25, 0.25, 0.25};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 4);
        int path[100];
        int codes[10][100];
        int codeCount = 0;
        Algorithms::huffmanGenerateCodes(root, path, 0, codes, codeCount);
        assert(codeCount == 4 && "Four codes generated");
        assert(is_prefix_free(codes, codeCount) && "Prefix-free property");
    }
    
    return 0;
}
