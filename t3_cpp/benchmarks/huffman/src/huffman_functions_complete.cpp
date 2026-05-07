#include "huffman_functions.h"
// Standard C++ Library (pre-included for agent evaluation)
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <queue>
#include <stack>
#include <deque>
#include <list>
#include <algorithm>
#include <cmath>
#include <string>
#include <memory>
#include <utility>


namespace Huffman {

std::unordered_map<unsigned char, int> build_frequency_table(const std::string &text) {
    // FUNCTION_ID: huffman_func001 - START
    std::unordered_map<unsigned char, int> frequency_table;
    for (unsigned char ch : text) {
        frequency_table[ch]++;
    }
    return frequency_table;
    // FUNCTION_ID: huffman_func001 - END
}

std::shared_ptr<Node> build_huffman_tree(const std::unordered_map<unsigned char, int> &frequency_table) {
    // FUNCTION_ID: huffman_func002 - START
    std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, Node::Compare> pq;

    for (const auto &entry : frequency_table) {
        pq.push(std::make_shared<Node>(entry.first, entry.second));
    }

    while (pq.size() > 1) {
        auto left = pq.top();
        pq.pop();
        auto right = pq.top();
        pq.pop();
        auto newNode = std::make_shared<Node>('+', left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }

    return pq.top();
    // FUNCTION_ID: huffman_func002 - END
}

void build_huffman_codes(const std::shared_ptr<Node> &root, const std::string &code, std::unordered_map<unsigned char, std::string> &huffman_codes) {
    // FUNCTION_ID: huffman_func003 - START
    if (!root->left && !root->right) {
        huffman_codes[root->character] = code;
        return;
    }

    if (root->left)
        build_huffman_codes(root->left, code + '0', huffman_codes);
    if (root->right)
        build_huffman_codes(root->right, code + '1', huffman_codes);
    // FUNCTION_ID: huffman_func003 - END
}

std::string encode_text(const std::string &text, const std::unordered_map<unsigned char, std::string> &huffman_codes) {
    // FUNCTION_ID: huffman_func004 - START
    std::string encoded;
    for (unsigned char ch : text) {
        encoded += huffman_codes.at(ch);
    }
    return encoded;
    // FUNCTION_ID: huffman_func004 - END
}

void write_huffman_dictionary(std::ofstream &outfile, const std::unordered_map<unsigned char, std::string> &huffman_codes) {
    // FUNCTION_ID: huffman_func005 - START
    char dict_size = static_cast<char>(huffman_codes.size());
    outfile.write(&dict_size, sizeof(char));

    for (const auto &entry : huffman_codes) {
        char character = static_cast<char>(entry.first);
        char length = static_cast<char>(entry.second.length());
        outfile.write(&character, sizeof(char));
        outfile.write(&length, sizeof(char));
        outfile.write(entry.second.c_str(), length);
    }
    // FUNCTION_ID: huffman_func005 - END
}

void read_huffman_dictionary(std::ifstream &infile, std::shared_ptr<Node> &root) {
    // FUNCTION_ID: huffman_func006 - START
    char dict_size;
    infile.read(&dict_size, sizeof(char));

    for (int i = 0; i < dict_size; ++i) {
        char character;
        char code_length;
        infile.read(&character, sizeof(char));
        infile.read(&code_length, sizeof(char));

        std::string code;
        code.resize(code_length);
        infile.read(&code[0], code_length);

        std::shared_ptr<Node> current = root;
        for (char bit : code) {
            if (bit == '0') {
                if (!current->left) {
                    current->left = std::make_shared<Node>('+', 0);
                }
                current = current->left;
            } else {
                if (!current->right) {
                    current->right = std::make_shared<Node>('+', 0);
                }
                current = current->right;
            }
        }
        current->character = static_cast<unsigned char>(character);
    }
    // FUNCTION_ID: huffman_func006 - END
}

std::string decode_data(std::ifstream &infile, std::shared_ptr<Node> &root, int encoded_length) {
    // FUNCTION_ID: huffman_func007 - START
    std::string decoded_text;
    std::shared_ptr<Node> current = root;

    char buffer;
    int bit_count = 0;
    while (infile.read(&buffer, sizeof(char))) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (buffer & (1 << i)) != 0;

            if (bit) {
                current = current->right;
            } else {
                current = current->left;
            }

            if (!current->left && !current->right) {
                decoded_text += current->character;
                current = root;
            }
            bit_count++;
            if (bit_count >= encoded_length)
                break;
        }
    }

    return decoded_text;
    // FUNCTION_ID: huffman_func007 - END
}

}