#include "../src/huffman_functions.h"
#include <cassert>
#include <set>
#include <functional>

int main() {
    // PROPERTY 1: All codes are unique (prefix-free property)
    auto check_prefix_free = [](const std::unordered_map<unsigned char, std::string>& codes) {
        std::vector<std::string> code_list;
        for (const auto& p : codes) code_list.push_back(p.second);
        
        for (size_t i = 0; i < code_list.size(); ++i) {
            for (size_t j = 0; j < code_list.size(); ++j) {
                if (i == j) continue;
                // No code should be a prefix of another
                bool is_prefix = (code_list[i].length() <= code_list[j].length() && 
                                  code_list[j].substr(0, code_list[i].length()) == code_list[i]);
                assert(!is_prefix && "Codes must be prefix-free (no code is prefix of another)");
            }
        }
    };
    
    // PROPERTY 2: All codes contain only '0' and '1'
    auto check_binary_codes = [](const std::unordered_map<unsigned char, std::string>& codes) {
        for (const auto& p : codes) {
            for (char c : p.second) {
                assert((c == '0' || c == '1') && "Codes must contain only '0' and '1'");
            }
        }
    };
    
    // PROPERTY 3: Number of codes = number of leaf nodes
    std::function<int(const std::shared_ptr<Huffman::Node>&)> count_leaves = 
        [&](const std::shared_ptr<Huffman::Node>& root) -> int {
        if (!root) return 0;
        if (!root->left && !root->right) return 1;
        return count_leaves(root->left) + count_leaves(root->right);
    };
    
    // PROPERTY 4: All leaf characters have codes
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
    
    // PROPERTY 5: Codes can reconstruct tree path
    auto verify_code_paths = [](const std::shared_ptr<Huffman::Node>& root,
                                 const std::unordered_map<unsigned char, std::string>& codes) {
        for (const auto& entry : codes) {
            unsigned char ch = entry.first;
            const std::string& code = entry.second;
            
            // Follow the code path in the tree
            auto current = root;
            for (char bit : code) {
                if (bit == '0') {
                    assert(current->left != nullptr && "Code path must exist in tree");
                    current = current->left;
                } else if (bit == '1') {
                    assert(current->right != nullptr && "Code path must exist in tree");
                    current = current->right;
                } else {
                    assert(false && "Invalid bit in code");
                }
            }
            
            // Must end at a leaf with the correct character
            assert(!current->left && !current->right && "Code must lead to a leaf node");
            assert(current->character == ch && "Leaf must contain the correct character");
        }
    };
    
    // Test 1: Simple tree with two nodes
    auto root = std::make_shared<Huffman::Node>('+', 5);
    root->left = std::make_shared<Huffman::Node>('a', 2);
    root->right = std::make_shared<Huffman::Node>('b', 3);
    
    std::unordered_map<unsigned char, std::string> codes;
    Huffman::build_huffman_codes(root, "", codes);
    
    assert(codes.size() == 2 && "Should have codes for 2 characters");
    assert(codes.count('a') > 0 && "Character 'a' must have a code");
    assert(codes.count('b') > 0 && "Character 'b' must have a code");
    assert(codes['a'].length() == 1 && "Code for 'a' should be 1 bit");
    assert(codes['b'].length() == 1 && "Code for 'b' should be 1 bit");
    assert(codes['a'] != codes['b'] && "Codes must be different");
    
    int leaf_count = count_leaves(root);
    assert((int)codes.size() == leaf_count && "Number of codes must equal number of leaves");
    
    check_prefix_free(codes);
    check_binary_codes(codes);
    verify_code_paths(root, codes);
    
    std::set<unsigned char> leaf_chars;
    collect_leaf_chars(root, leaf_chars);
    for (unsigned char ch : leaf_chars) {
        assert(codes.count(ch) > 0 && "All leaf characters must have codes");
    }
    
    // Test 2: Single leaf node
    auto single_root = std::make_shared<Huffman::Node>('x', 1);
    std::unordered_map<unsigned char, std::string> single_codes;
    Huffman::build_huffman_codes(single_root, "", single_codes);
    
    assert(single_codes.size() == 1 && "Should have code for 1 character");
    assert(single_codes.count('x') > 0 && "Character 'x' must have a code");
    // Single character can have empty code or any code depending on implementation
    check_binary_codes(single_codes);
    
    // Test 3: Deeper tree (3 levels)
    auto root3 = std::make_shared<Huffman::Node>('+', 10);
    root3->left = std::make_shared<Huffman::Node>('a', 4);
    root3->right = std::make_shared<Huffman::Node>('+', 6);
    root3->right->left = std::make_shared<Huffman::Node>('b', 2);
    root3->right->right = std::make_shared<Huffman::Node>('c', 4);
    
    std::unordered_map<unsigned char, std::string> codes3;
    Huffman::build_huffman_codes(root3, "", codes3);
    
    assert(codes3.size() == 3 && "Should have codes for 3 characters");
    assert(codes3.count('a') && codes3.count('b') && codes3.count('c') && 
           "All characters must have codes");
    
    // Verify code lengths match tree depth
    assert(codes3['a'].length() == 1 && "'a' is at depth 1");
    assert(codes3['b'].length() == 2 && "'b' is at depth 2");
    assert(codes3['c'].length() == 2 && "'c' is at depth 2");
    
    check_prefix_free(codes3);
    check_binary_codes(codes3);
    verify_code_paths(root3, codes3);
    
    // Test 4: Unbalanced tree
    auto root4 = std::make_shared<Huffman::Node>('+', 15);
    root4->left = std::make_shared<Huffman::Node>('+', 6);
    root4->left->left = std::make_shared<Huffman::Node>('d', 2);
    root4->left->right = std::make_shared<Huffman::Node>('e', 4);
    root4->right = std::make_shared<Huffman::Node>('f', 9);
    
    std::unordered_map<unsigned char, std::string> codes4;
    Huffman::build_huffman_codes(root4, "", codes4);
    
    assert(codes4.size() == 3 && "Should have codes for 3 characters");
    assert(codes4['d'].length() == 2 && "'d' is at depth 2");
    assert(codes4['e'].length() == 2 && "'e' is at depth 2");
    assert(codes4['f'].length() == 1 && "'f' is at depth 1");
    
    check_prefix_free(codes4);
    check_binary_codes(codes4);
    verify_code_paths(root4, codes4);
    
    // Test 5: Binary/non-ASCII characters
    auto root5 = std::make_shared<Huffman::Node>('+', 10);
    root5->left = std::make_shared<Huffman::Node>(0, 3);    // null byte
    root5->right = std::make_shared<Huffman::Node>(255, 7); // 0xFF
    
    std::unordered_map<unsigned char, std::string> codes5;
    Huffman::build_huffman_codes(root5, "", codes5);
    
    assert(codes5.size() == 2 && "Should have codes for 2 binary characters");
    assert(codes5.count(0) && codes5.count(255) && "Both binary characters must have codes");
    assert(codes5[0].length() == 1 && codes5[255].length() == 1 && 
           "Both codes should be 1 bit");
    
    check_prefix_free(codes5);
    check_binary_codes(codes5);
    verify_code_paths(root5, codes5);
    
    // Test 6: Large tree (7 leaves)
    auto root6 = std::make_shared<Huffman::Node>('+', 100);
    root6->left = std::make_shared<Huffman::Node>('+', 40);
    root6->left->left = std::make_shared<Huffman::Node>('g', 15);
    root6->left->right = std::make_shared<Huffman::Node>('+', 25);
    root6->left->right->left = std::make_shared<Huffman::Node>('h', 10);
    root6->left->right->right = std::make_shared<Huffman::Node>('i', 15);
    root6->right = std::make_shared<Huffman::Node>('+', 60);
    root6->right->left = std::make_shared<Huffman::Node>('+', 25);
    root6->right->left->left = std::make_shared<Huffman::Node>('j', 10);
    root6->right->left->right = std::make_shared<Huffman::Node>('k', 15);
    root6->right->right = std::make_shared<Huffman::Node>('+', 35);
    root6->right->right->left = std::make_shared<Huffman::Node>('l', 15);
    root6->right->right->right = std::make_shared<Huffman::Node>('m', 20);
    
    std::unordered_map<unsigned char, std::string> codes6;
    Huffman::build_huffman_codes(root6, "", codes6);
    
    assert(codes6.size() == 7 && "Should have codes for 7 characters");
    check_prefix_free(codes6);
    check_binary_codes(codes6);
    verify_code_paths(root6, codes6);
    
    // Verify code lengths match tree structure (not exact values)
    assert(codes6['g'].length() == 2 && "'g' is at depth 2");
    assert(codes6['h'].length() == 3 && "'h' is at depth 3");
    assert(codes6['i'].length() == 3 && "'i' is at depth 3");
    assert(codes6['j'].length() == 3 && "'j' is at depth 3");
    assert(codes6['k'].length() == 3 && "'k' is at depth 3");
    assert(codes6['l'].length() == 3 && "'l' is at depth 3");
    assert(codes6['m'].length() == 3 && "'m' is at depth 3");
    
    return 0;
}