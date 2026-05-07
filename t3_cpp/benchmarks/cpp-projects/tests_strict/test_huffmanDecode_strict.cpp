#include "../src/ds_algorithms.h"
#include <cassert>

int main() {
    // Test 1: Simple decode
    {
        int values[] = {97, 98};
        double freqs[] = {0.6, 0.4};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 2);
        int encoded[] = {0, 1, 0};
        int decoded[10];
        int decLen;
        Algorithms::huffmanDecode(root, encoded, 3, decoded, decLen);
        assert(decLen > 0 && "Decoded some values");
    }
    
    // Test 2: Empty encoding
    {
        int values[] = {97, 98};
        double freqs[] = {0.6, 0.4};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 2);
        int encoded[] = {};
        int decoded[10];
        int decLen;
        Algorithms::huffmanDecode(root, encoded, 0, decoded, decLen);
        assert(decLen == 0 && "No values decoded");
    }
    
    // Test 3: Multiple symbols
    {
        int values[] = {97, 98, 99};
        double freqs[] = {0.5, 0.3, 0.2};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 3);
        int path[100];
        int codes[10][100];
        int codeCount = 0;
        Algorithms::huffmanGenerateCodes(root, path, 0, codes, codeCount);
        
        // Create encoded message
        int encoded[100];
        int encLen = 0;
        for (int i = 0; codes[0][i] != -1; i++) {
            encoded[encLen++] = codes[0][i];
        }
        
        int decoded[10];
        int decLen;
        Algorithms::huffmanDecode(root, encoded, encLen, decoded, decLen);
        assert(decLen >= 1 && "At least one symbol decoded");
    }
    
    return 0;
}
