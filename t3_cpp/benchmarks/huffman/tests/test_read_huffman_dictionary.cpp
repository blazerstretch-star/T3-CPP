#include "../src/huffman_functions.h"
#include <cassert>
#include <sstream>
#include <cstdio>
#include <set>
#include <functional>

int main() {
    // PROPERTY 1: Tree structure matches dictionary (all codes lead to leaves)
    auto verify_tree_structure = [](const std::shared_ptr<Huffman::Node>& root,
                                    const std::unordered_map<unsigned char, std::string>& expected_codes) {
        // Traverse tree and collect codes
        std::unordered_map<unsigned char, std::string> tree_codes;
        std::function<void(const std::shared_ptr<Huffman::Node>&, std::string)> traverse = 
            [&](const std::shared_ptr<Huffman::Node>& node, std::string code) {
            if (!node) return;
            if (!node->left && !node->right) {
                tree_codes[node->character] = code;
            }
            if (node->left) traverse(node->left, code + "0");
            if (node->right) traverse(node->right, code + "1");
        };
        traverse(root, "");
        
        // Verify all expected codes are in tree
        for (const auto& entry : expected_codes) {
            assert(tree_codes.count(entry.first) > 0 && "All dictionary characters must be in tree");
            assert(tree_codes[entry.first] == entry.second && "Tree codes must match dictionary codes");
        }
    };
    
    // PROPERTY 2: All internal nodes have exactly 2 children or 0 children
    auto check_binary_tree = [](const std::shared_ptr<Huffman::Node>& root) {
        std::function<void(const std::shared_ptr<Huffman::Node>&)> validate = 
            [&](const std::shared_ptr<Huffman::Node>& node) {
            if (!node) return;
            bool has_left = (node->left != nullptr);
            bool has_right = (node->right != nullptr);
            // Either both children or no children
            assert((has_left == has_right) && "Internal nodes must have 0 or 2 children");
            validate(node->left);
            validate(node->right);
        };
        validate(root);
    };
    
    // PROPERTY 3: Number of leaf nodes = dictionary size
    std::function<int(const std::shared_ptr<Huffman::Node>&)> count_leaves = 
        [&](const std::shared_ptr<Huffman::Node>& root) -> int {
        if (!root) return 0;
        if (!root->left && !root->right) return 1;
        return count_leaves(root->left) + count_leaves(root->right);
    };
    
    // Test 1: Create a test dictionary file
    std::ofstream outfile("test_read_dict.bin", std::ios::binary);
    char dict_size = 2;
    outfile.write(&dict_size, sizeof(char));
    
    // Write 'a' -> "0"
    char char_a = 'a', len_a = 1;
    outfile.write(&char_a, sizeof(char));
    outfile.write(&len_a, sizeof(char));
    outfile.write("0", 1);
    
    // Write 'b' -> "1"
    char char_b = 'b', len_b = 1;
    outfile.write(&char_b, sizeof(char));
    outfile.write(&len_b, sizeof(char));
    outfile.write("1", 1);
    outfile.close();
    
    // Test reading the dictionary
    std::ifstream infile("test_read_dict.bin", std::ios::binary);
    auto root = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(infile, root);
    infile.close();
    
    // Verify tree structure
    assert(root->left != nullptr && "Root should have left child");
    assert(root->right != nullptr && "Root should have right child");
    assert(root->left->character == 'a' && "Left child should be 'a'");
    assert(root->right->character == 'b' && "Right child should be 'b'");
    assert(root->left->left == nullptr && root->left->right == nullptr && "Left child should be leaf");
    assert(root->right->left == nullptr && root->right->right == nullptr && "Right child should be leaf");
    
    std::unordered_map<unsigned char, std::string> expected1 = {{'a', "0"}, {'b', "1"}};
    verify_tree_structure(root, expected1);
    check_binary_tree(root);
    assert(count_leaves(root) == 2 && "Should have 2 leaf nodes");
    
    std::remove("test_read_dict.bin");
    
    // Test 2: Empty dictionary
    std::ofstream out2("test_empty.bin", std::ios::binary);
    char empty_size = 0;
    out2.write(&empty_size, sizeof(char));
    out2.close();
    
    std::ifstream in2("test_empty.bin", std::ios::binary);
    auto root2 = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(in2, root2);
    in2.close();
    
    // Empty dictionary leaves root unchanged (still has no children)
    assert(!root2->left && !root2->right && "Empty dictionary should not add children to root");
    std::remove("test_empty.bin");
    
    // Test 3: Single character with empty code
    std::ofstream out3("test_single.bin", std::ios::binary);
    char size3 = 1;
    out3.write(&size3, sizeof(char));
    char char_x = 'x', len_x = 0;
    out3.write(&char_x, sizeof(char));
    out3.write(&len_x, sizeof(char));
    out3.close();
    
    std::ifstream in3("test_single.bin", std::ios::binary);
    auto root3 = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(in3, root3);
    in3.close();
    
    assert(root3->character == 'x' && "Root should be 'x' for single character");
    std::remove("test_single.bin");
    
    // Test 4: Deeper tree (3 levels)
    std::ofstream out4("test_deep.bin", std::ios::binary);
    char size4 = 3;
    out4.write(&size4, sizeof(char));
    
    // 'a' -> "0"
    char ch_a = 'a', len_a4 = 1;
    out4.write(&ch_a, sizeof(char));
    out4.write(&len_a4, sizeof(char));
    out4.write("0", 1);
    
    // 'b' -> "10"
    char ch_b = 'b', len_b4 = 2;
    out4.write(&ch_b, sizeof(char));
    out4.write(&len_b4, sizeof(char));
    out4.write("10", 2);
    
    // 'c' -> "11"
    char ch_c = 'c', len_c4 = 2;
    out4.write(&ch_c, sizeof(char));
    out4.write(&len_c4, sizeof(char));
    out4.write("11", 2);
    out4.close();
    
    std::ifstream in4("test_deep.bin", std::ios::binary);
    auto root4 = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(in4, root4);
    in4.close();
    
    std::unordered_map<unsigned char, std::string> expected4 = {{'a', "0"}, {'b', "10"}, {'c', "11"}};
    verify_tree_structure(root4, expected4);
    check_binary_tree(root4);
    assert(count_leaves(root4) == 3 && "Should have 3 leaf nodes");
    
    std::remove("test_deep.bin");
    
    // Test 5: Binary/non-ASCII characters
    std::ofstream out5("test_binary.bin", std::ios::binary);
    char size5 = 3;
    out5.write(&size5, sizeof(char));
    
    char ch_null = 0, len_null = 2;
    out5.write(&ch_null, sizeof(char));
    out5.write(&len_null, sizeof(char));
    out5.write("00", 2);
    
    char ch_ff = -1, len_ff = 2; // 255 as signed char
    out5.write(&ch_ff, sizeof(char));
    out5.write(&len_ff, sizeof(char));
    out5.write("01", 2);
    
    char ch_80 = -128, len_80 = 1; // 128 as signed char
    out5.write(&ch_80, sizeof(char));
    out5.write(&len_80, sizeof(char));
    out5.write("1", 1);
    out5.close();
    
    std::ifstream in5("test_binary.bin", std::ios::binary);
    auto root5 = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(in5, root5);
    in5.close();
    
    std::unordered_map<unsigned char, std::string> expected5 = {{0, "00"}, {255, "01"}, {128, "1"}};
    verify_tree_structure(root5, expected5);
    check_binary_tree(root5);
    
    std::remove("test_binary.bin");
    
    // Test 6: Round-trip with write_huffman_dictionary
    std::unordered_map<unsigned char, std::string> codes6 = {
        {'p', "00"}, {'q', "01"}, {'r', "10"}, {'s', "11"}
    };
    
    std::ofstream out6("test_roundtrip.bin", std::ios::binary);
    Huffman::write_huffman_dictionary(out6, codes6);
    out6.close();
    
    std::ifstream in6("test_roundtrip.bin", std::ios::binary);
    auto root6 = std::make_shared<Huffman::Node>('+', 0);
    Huffman::read_huffman_dictionary(in6, root6);
    in6.close();
    
    verify_tree_structure(root6, codes6);
    check_binary_tree(root6);
    assert(count_leaves(root6) == 4 && "Should have 4 leaf nodes");
    
    std::remove("test_roundtrip.bin");
    
    return 0;
}