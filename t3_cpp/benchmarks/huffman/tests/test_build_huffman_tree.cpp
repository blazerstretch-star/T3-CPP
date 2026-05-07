#include "../src/huffman_functions.h"
#include <cassert>
#include <queue>
#include <set>
#include <functional>

int main() {
    // PROPERTY 1: Root frequency = sum of all input frequencies
    auto check_frequency_sum = [](const std::unordered_map<unsigned char, int>& freq, 
                                   const std::shared_ptr<Huffman::Node>& root) {
        int expected_sum = 0;
        for (const auto& p : freq) expected_sum += p.second;
        assert(root->frequency == expected_sum && "Root frequency must equal sum of all frequencies");
    };
    
    // PROPERTY 2: Tree has exactly (2n-1) nodes for n leaf nodes
    std::function<int(const std::shared_ptr<Huffman::Node>&)> count_nodes = 
        [&](const std::shared_ptr<Huffman::Node>& root) -> int {
        if (!root) return 0;
        return 1 + count_nodes(root->left) + count_nodes(root->right);
    };
    
    // PROPERTY 3: All leaf nodes contain original characters
    auto collect_leaf_chars = [](const std::shared_ptr<Huffman::Node>& root, std::set<unsigned char>& chars) {
        std::function<void(const std::shared_ptr<Huffman::Node>&)> traverse = 
            [&](const std::shared_ptr<Huffman::Node>& node) {
            if (!node) return;
            if (!node->left && !node->right) {
                chars.insert(node->character);
            }
            traverse(node->left);
            traverse(node->right);
        };
        traverse(root);
    };
    
    // PROPERTY 4: Parent frequency >= child frequencies (min-heap property)
    auto check_heap_property = [](const std::shared_ptr<Huffman::Node>& root) {
        std::function<void(const std::shared_ptr<Huffman::Node>&)> validate = 
            [&](const std::shared_ptr<Huffman::Node>& node) {
            if (!node) return;
            if (node->left) {
                assert(node->frequency >= node->left->frequency && 
                       "Parent frequency must be >= left child frequency");
                validate(node->left);
            }
            if (node->right) {
                assert(node->frequency >= node->right->frequency && 
                       "Parent frequency must be >= right child frequency");
                validate(node->right);
            }
        };
        validate(root);
    };
    
    // Test 1: Simple frequency table
    std::unordered_map<unsigned char, int> freq1 = {{'a', 3}, {'b', 1}, {'c', 2}};
    auto root1 = Huffman::build_huffman_tree(freq1);
    assert(root1 != nullptr && "Root should not be null");
    check_frequency_sum(freq1, root1);
    
    int node_count1 = count_nodes(root1);
    assert(node_count1 == 5 && "Tree with 3 leaves should have 5 nodes (2*3-1)");
    
    std::set<unsigned char> leaf_chars1;
    collect_leaf_chars(root1, leaf_chars1);
    assert(leaf_chars1.size() == 3 && "Should have 3 leaf characters");
    assert(leaf_chars1.count('a') && leaf_chars1.count('b') && leaf_chars1.count('c') && 
           "All original characters should be in leaves");
    
    check_heap_property(root1);
    
    // Test 2: Single character
    std::unordered_map<unsigned char, int> freq2 = {{'x', 5}};
    auto root2 = Huffman::build_huffman_tree(freq2);
    assert(root2 != nullptr && "Root should not be null");
    assert(root2->frequency == 5 && "Root frequency should be 5");
    assert(root2->character == 'x' && "Root character should be 'x'");
    assert(!root2->left && !root2->right && "Single node tree should have no children");
    
    int node_count2 = count_nodes(root2);
    assert(node_count2 == 1 && "Single character tree should have 1 node");
    
    // Test 3: Two characters
    std::unordered_map<unsigned char, int> freq3 = {{'p', 2}, {'q', 3}};
    auto root3 = Huffman::build_huffman_tree(freq3);
    assert(root3 != nullptr && "Root should not be null");
    check_frequency_sum(freq3, root3);
    
    // For two characters, tree must have exactly 3 nodes (root + 2 leaves)
    int node_count3 = count_nodes(root3);
    assert(node_count3 == 3 && "Tree with 2 leaves should have 3 nodes (2*2-1)");
    
    // Root must have two children (both leaves)
    assert(root3->left != nullptr && root3->right != nullptr && "Root should have two children");
    assert((!root3->left->left && !root3->left->right) && "Left child should be a leaf");
    assert((!root3->right->left && !root3->right->right) && "Right child should be a leaf");
    
    // Verify both characters are in the tree
    std::set<unsigned char> chars3;
    chars3.insert(root3->left->character);
    chars3.insert(root3->right->character);
    assert(chars3.count('p') && chars3.count('q') && "Both characters must be in tree");
    
    check_heap_property(root3);
    
    // Test 4: Many characters with varying frequencies
    std::unordered_map<unsigned char, int> freq4 = {
        {'a', 45}, {'b', 13}, {'c', 12}, {'d', 16}, {'e', 9}, {'f', 5}
    };
    auto root4 = Huffman::build_huffman_tree(freq4);
    assert(root4 != nullptr && "Root should not be null");
    check_frequency_sum(freq4, root4);
    
    int node_count4 = count_nodes(root4);
    assert(node_count4 == 11 && "Tree with 6 leaves should have 11 nodes (2*6-1)");
    
    std::set<unsigned char> leaf_chars4;
    collect_leaf_chars(root4, leaf_chars4);
    assert(leaf_chars4.size() == 6 && "Should have 6 leaf characters");
    
    check_heap_property(root4);
    
    // Test 5: Equal frequencies (tests tie-breaking)
    std::unordered_map<unsigned char, int> freq5 = {{'a', 1}, {'b', 1}, {'c', 1}, {'d', 1}};
    auto root5 = Huffman::build_huffman_tree(freq5);
    assert(root5 != nullptr && "Root should not be null");
    check_frequency_sum(freq5, root5);
    
    int node_count5 = count_nodes(root5);
    assert(node_count5 == 7 && "Tree with 4 leaves should have 7 nodes (2*4-1)");
    
    check_heap_property(root5);
    
    // Test 6: Binary characters (non-ASCII)
    std::unordered_map<unsigned char, int> freq6 = {{0, 10}, {255, 20}, {128, 15}};
    auto root6 = Huffman::build_huffman_tree(freq6);
    assert(root6 != nullptr && "Root should not be null");
    check_frequency_sum(freq6, root6);
    
    std::set<unsigned char> leaf_chars6;
    collect_leaf_chars(root6, leaf_chars6);
    assert(leaf_chars6.count(0) && leaf_chars6.count(255) && leaf_chars6.count(128) && 
           "All binary characters should be in leaves");
    
    check_heap_property(root6);
    
    // Test 7: Large frequency values
    std::unordered_map<unsigned char, int> freq7 = {{'x', 1000000}, {'y', 500000}};
    auto root7 = Huffman::build_huffman_tree(freq7);
    assert(root7 != nullptr && "Root should not be null");
    assert(root7->frequency == 1500000 && "Root frequency should handle large values");
    check_heap_property(root7);
    
    return 0;
}