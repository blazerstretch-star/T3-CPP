#include "huffman_tree.h"

namespace huffman
{

    std::shared_ptr<Node> build_huffman_tree(const std::unordered_map<unsigned char, int> &frequency_table)
    {
        // FUNCTION_ID: huffman_func002 - START
        std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, Node::Compare> pq;

        for (const auto &entry : frequency_table)
        {
            pq.push(std::make_shared<Node>(entry.first, entry.second));
        }

        while (pq.size() > 1)
        {
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

    void build_huffman_codes(const std::shared_ptr<Node> &root, const std::string &code,
                             std::unordered_map<unsigned char, std::string> &huffman_codes)
    {
        // FUNCTION_ID: huffman_func003 - START
        if (!root->left && !root->right)
        {
            huffman_codes[root->character] = code;
            return;
        }

        if (root->left)
            build_huffman_codes(root->left, code + '0', huffman_codes);
        if (root->right)
            build_huffman_codes(root->right, code + '1', huffman_codes);
        // FUNCTION_ID: huffman_func003 - END
    }

} // namespace huffman