#include "../src/huffman_functions.h"
#include <cassert>
#include <cstdio>

int main() {
    // PROPERTY 1: Decoded length matches expected character count
    auto check_decoded_length = [](const std::string& decoded, int expected_chars) {
        assert((int)decoded.length() == expected_chars && 
               "Decoded text length must match expected character count");
    };
    
    // PROPERTY 2: Round-trip consistency (encode -> decode = original)
    auto check_round_trip = [](const std::shared_ptr<Huffman::Node>& root,
                               const std::unordered_map<unsigned char, std::string>& codes,
                               const std::string& original_text) {
        // Encode
        std::string encoded = Huffman::encode_text(original_text, codes);
        
        // Write encoded bits to file
        std::ofstream outfile("test_roundtrip_decode.bin", std::ios::binary);
        for (size_t i = 0; i < encoded.length(); i += 8) {
            char byte = 0;
            for (int j = 0; j < 8 && i + j < encoded.length(); ++j) {
                if (encoded[i + j] == '1') {
                    byte |= (1 << (7 - j));
                }
            }
            outfile.write(&byte, sizeof(char));
        }
        outfile.close();
        
        // Decode
        std::ifstream infile("test_roundtrip_decode.bin", std::ios::binary);
        std::string decoded = Huffman::decode_data(infile, const_cast<std::shared_ptr<Huffman::Node>&>(root), encoded.length());
        infile.close();
        
        assert(decoded == original_text && "Round-trip encode/decode must preserve original text");
        std::remove("test_roundtrip_decode.bin");
    };
    
    // PROPERTY 3: Decoding stops at exact bit count
    auto check_bit_boundary = [](const std::shared_ptr<Huffman::Node>& root, int bit_count) {
        std::ofstream outfile("test_boundary.bin", std::ios::binary);
        char byte = 0xFF; // All 1s
        outfile.write(&byte, sizeof(char));
        outfile.close();
        
        std::ifstream infile("test_boundary.bin", std::ios::binary);
        std::string decoded = Huffman::decode_data(infile, const_cast<std::shared_ptr<Huffman::Node>&>(root), bit_count);
        infile.close();
        
        // Should decode exactly bit_count bits, no more
        assert((int)decoded.length() <= bit_count && "Should not decode beyond specified bit count");
        std::remove("test_boundary.bin");
    };
    
    // Test 1: Simple tree - use actual codes from tree
    auto root = std::make_shared<Huffman::Node>('+', 0);
    root->left = std::make_shared<Huffman::Node>('a', 0);
    root->right = std::make_shared<Huffman::Node>('b', 0);
    
    // Generate codes from tree to know what encoding to use
    std::unordered_map<unsigned char, std::string> codes1;
    Huffman::build_huffman_codes(root, "", codes1);
    
    // Encode "aba" using actual codes
    std::string text1 = "aba";
    std::string encoded1 = Huffman::encode_text(text1, codes1);
    
    // Write encoded bits to file
    std::ofstream outfile("test_decode.bin", std::ios::binary);
    for (size_t i = 0; i < encoded1.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded1.length(); ++j) {
            if (encoded1[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile.write(&byte, sizeof(char));
    }
    outfile.close();
    
    // Test decoding
    std::ifstream infile("test_decode.bin", std::ios::binary);
    std::string decoded = Huffman::decode_data(infile, root, encoded1.length());
    infile.close();
    
    assert(decoded == text1 && "Decoded text should match original");
    check_decoded_length(decoded, 3);
    
    std::remove("test_decode.bin");
    
    // Test 2: Empty file
    std::ofstream outfile2("test_decode2.bin", std::ios::binary);
    outfile2.close(); // Empty file
    
    std::ifstream infile2("test_decode2.bin", std::ios::binary);
    std::string decoded2 = Huffman::decode_data(infile2, root, 0);
    infile2.close();
    
    assert(decoded2.empty() && "Decoded text should be empty for 0 length");
    std::remove("test_decode2.bin");
    
    // Test 3: Longer sequence - use actual codes
    std::string text3 = "abababab";
    std::string encoded3 = Huffman::encode_text(text3, codes1);
    
    std::ofstream outfile3("test_decode3.bin", std::ios::binary);
    for (size_t i = 0; i < encoded3.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded3.length(); ++j) {
            if (encoded3[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile3.write(&byte, sizeof(char));
    }
    outfile3.close();
    
    std::ifstream infile3("test_decode3.bin", std::ios::binary);
    std::string decoded3 = Huffman::decode_data(infile3, root, encoded3.length());
    infile3.close();
    
    assert(decoded3 == text3 && "Decoded text should match original");
    check_decoded_length(decoded3, 8);
    
    std::remove("test_decode3.bin");
    
    // Test 4: Variable length codes - use actual codes from tree
    auto root4 = std::make_shared<Huffman::Node>('+', 0);
    root4->left = std::make_shared<Huffman::Node>('x', 0);
    root4->right = std::make_shared<Huffman::Node>('+', 0);
    root4->right->left = std::make_shared<Huffman::Node>('y', 0);
    root4->right->right = std::make_shared<Huffman::Node>('z', 0);
    
    std::unordered_map<unsigned char, std::string> codes4;
    Huffman::build_huffman_codes(root4, "", codes4);
    
    std::string text4 = "xyz";
    std::string encoded4 = Huffman::encode_text(text4, codes4);
    
    std::ofstream outfile4("test_decode4.bin", std::ios::binary);
    for (size_t i = 0; i < encoded4.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded4.length(); ++j) {
            if (encoded4[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile4.write(&byte, sizeof(char));
    }
    outfile4.close();
    
    std::ifstream infile4("test_decode4.bin", std::ios::binary);
    std::string decoded4 = Huffman::decode_data(infile4, root4, encoded4.length());
    infile4.close();
    
    assert(decoded4 == text4 && "Decoded text should match original");
    check_decoded_length(decoded4, 3);
    
    std::remove("test_decode4.bin");
    
    // Test 5: Multiple bytes - use actual codes
    std::string text5(16, 'a');
    for (int i = 0; i < 16; i += 2) text5[i] = 'b'; // Alternate pattern
    std::string encoded5 = Huffman::encode_text(text5, codes1);
    
    std::ofstream outfile5("test_decode5.bin", std::ios::binary);
    for (size_t i = 0; i < encoded5.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded5.length(); ++j) {
            if (encoded5[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile5.write(&byte, sizeof(char));
    }
    outfile5.close();
    
    std::ifstream infile5("test_decode5.bin", std::ios::binary);
    std::string decoded5 = Huffman::decode_data(infile5, root, encoded5.length());
    infile5.close();
    
    assert(decoded5 == text5 && "Decoded text should match original");
    assert(decoded5.length() == 16 && "Should decode 16 characters");
    std::remove("test_decode5.bin");
    
    // Test 6: Round-trip with simple codes
    std::unordered_map<unsigned char, std::string> codes6 = {{'a', "0"}, {'b', "1"}};
    check_round_trip(root, codes6, "aabba");
    check_round_trip(root, codes6, "a");
    check_round_trip(root, codes6, "b");
    check_round_trip(root, codes6, "");
    
    // Test 7: Binary/non-ASCII characters - use actual codes
    auto root7 = std::make_shared<Huffman::Node>('+', 0);
    root7->left = std::make_shared<Huffman::Node>(0, 0);   // null byte
    root7->right = std::make_shared<Huffman::Node>(255, 0); // 0xFF
    
    std::unordered_map<unsigned char, std::string> codes7;
    Huffman::build_huffman_codes(root7, "", codes7);
    
    std::string text7;
    text7.push_back('\x00');
    text7.push_back('\xFF');
    text7.push_back('\xFF');
    std::string encoded7 = Huffman::encode_text(text7, codes7);
    
    std::ofstream outfile7("test_decode7.bin", std::ios::binary);
    for (size_t i = 0; i < encoded7.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded7.length(); ++j) {
            if (encoded7[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile7.write(&byte, sizeof(char));
    }
    outfile7.close();
    
    std::ifstream infile7("test_decode7.bin", std::ios::binary);
    std::string decoded7 = Huffman::decode_data(infile7, root7, encoded7.length());
    infile7.close();
    
    assert(decoded7.length() == 3 && "Should decode 3 binary characters");
    assert(decoded7 == text7 && "Decoded binary text should match original");
    
    std::remove("test_decode7.bin");
    
    // Test 8: Partial byte (bit count not multiple of 8) - use actual codes
    std::string text8 = "ab";
    std::string encoded8 = Huffman::encode_text(text8, codes1);
    
    std::ofstream outfile8("test_decode8.bin", std::ios::binary);
    for (size_t i = 0; i < encoded8.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded8.length(); ++j) {
            if (encoded8[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile8.write(&byte, sizeof(char));
    }
    outfile8.close();
    
    std::ifstream infile8("test_decode8.bin", std::ios::binary);
    std::string decoded8 = Huffman::decode_data(infile8, root, encoded8.length());
    infile8.close();
    
    assert(decoded8 == text8 && "Decoded text should match original");
    check_decoded_length(decoded8, 2);
    
    std::remove("test_decode8.bin");
    
    // Test 9: Large data - use actual codes
    std::string text9(800, 'a');
    for (int i = 0; i < 800; i += 2) text9[i] = 'b';
    std::string encoded9 = Huffman::encode_text(text9, codes1);
    
    std::ofstream outfile9("test_decode9.bin", std::ios::binary);
    for (size_t i = 0; i < encoded9.length(); i += 8) {
        char byte = 0;
        for (int j = 0; j < 8 && i + j < encoded9.length(); ++j) {
            if (encoded9[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outfile9.write(&byte, sizeof(char));
    }
    outfile9.close();
    
    std::ifstream infile9("test_decode9.bin", std::ios::binary);
    std::string decoded9 = Huffman::decode_data(infile9, root, encoded9.length());
    infile9.close();
    
    assert(decoded9 == text9 && "Decoded text should match original");
    assert(decoded9.length() == 800 && "Should decode 800 characters");
    std::remove("test_decode9.bin");
    
    return 0;
}