#pragma once
#include <memory>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <fstream>

namespace Huffman {

class Node {
public:
    unsigned char character;
    int frequency;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    Node(unsigned char ch, int freq) : character(ch), frequency(freq), left(nullptr), right(nullptr) {}

    struct Compare {
        bool operator()(std::shared_ptr<Node> const &l, std::shared_ptr<Node> const &r) {
            return l->frequency > r->frequency;
        }
    };
};

// Function declarations
std::unordered_map<unsigned char, int> build_frequency_table(const std::string &text);
std::shared_ptr<Node> build_huffman_tree(const std::unordered_map<unsigned char, int> &frequency_table);
void build_huffman_codes(const std::shared_ptr<Node> &root, const std::string &code, std::unordered_map<unsigned char, std::string> &huffman_codes);
std::string encode_text(const std::string &text, const std::unordered_map<unsigned char, std::string> &huffman_codes);
void write_huffman_dictionary(std::ofstream &outfile, const std::unordered_map<unsigned char, std::string> &huffman_codes);
void read_huffman_dictionary(std::ifstream &infile, std::shared_ptr<Node> &root);
std::string decode_data(std::ifstream &infile, std::shared_ptr<Node> &root, int encoded_length);

// Additional utility functions
bool validate_huffman_tree(const std::shared_ptr<Node> &root);
int calculate_tree_depth(const std::shared_ptr<Node> &root);
double calculate_compression_ratio(const std::string &original_text, const std::unordered_map<unsigned char, std::string> &huffman_codes);
std::vector<unsigned char> write_bits_to_bytes(const std::string &bit_string);
std::string serialize_tree(const std::shared_ptr<Node> &root);

}