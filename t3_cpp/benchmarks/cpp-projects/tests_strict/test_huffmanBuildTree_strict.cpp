#include "../src/ds_algorithms.h"
#include <cassert>
#include <cmath>
#include <functional>

int main() {
    std::function<int(TreeNode*)> count_nodes = [&](TreeNode* node) -> int {
        if (!node) return 0;
        return 1 + count_nodes(node->left) + count_nodes(node->right);
    };
    
    std::function<int(TreeNode*)> count_leaves = [&](TreeNode* node) -> int {
        if (!node) return 0;
        if (!node->left && !node->right) return 1;
        return count_leaves(node->left) + count_leaves(node->right);
    };
    
    std::function<double(TreeNode*)> sum_frequencies = [&](TreeNode* node) -> double {
        if (!node) return 0.0;
        if (!node->left && !node->right) return node->frequency;
        return sum_frequencies(node->left) + sum_frequencies(node->right);
    };
    
    const double EPSILON = 1e-9;
    
    // Test 1: Single value
    {
        int values[] = {97};
        double freqs[] = {1.0};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 1);
        assert(root != nullptr && "Tree created");
        assert(count_nodes(root) == 1 && "One node");
    }
    
    // Test 2: Two values
    {
        int values[] = {97, 98};
        double freqs[] = {0.6, 0.4};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 2);
        assert(root != nullptr && "Tree created");
        assert(count_leaves(root) == 2 && "Two leaves");
        double total = sum_frequencies(root);
        assert(std::abs(total - 1.0) < EPSILON && "Frequencies sum to 1.0");
    }
    
    // Test 3: Three values
    {
        int values[] = {97, 98, 99};
        double freqs[] = {0.5, 0.3, 0.2};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 3);
        assert(root != nullptr && "Tree created");
        assert(count_leaves(root) == 3 && "Three leaves");
        double total = sum_frequencies(root);
        assert(std::abs(total - 1.0) < EPSILON && "Frequencies sum to 1.0");
    }
    
    // Test 4: Equal frequencies
    {
        int values[] = {1, 2, 3, 4};
        double freqs[] = {0.25, 0.25, 0.25, 0.25};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 4);
        assert(root != nullptr && "Tree created");
        assert(count_leaves(root) == 4 && "Four leaves");
    }
    
    // Test 5: Skewed frequencies
    {
        int values[] = {1, 2, 3};
        double freqs[] = {0.8, 0.15, 0.05};
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 3);
        assert(root != nullptr && "Tree created");
        assert(count_leaves(root) == 3 && "Three leaves");
    }
    
    // Test 6: Many values
    {
        int values[10];
        double freqs[10];
        for (int i = 0; i < 10; i++) {
            values[i] = i;
            freqs[i] = 0.1;
        }
        TreeNode* root = Algorithms::huffmanBuildTree(values, freqs, 10);
        assert(root != nullptr && "Tree created");
        assert(count_leaves(root) == 10 && "Ten leaves");
    }
    
    return 0;
}
