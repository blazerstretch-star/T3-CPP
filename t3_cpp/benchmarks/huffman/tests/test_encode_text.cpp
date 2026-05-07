#include "../src/huffman_functions.h"
#include <cassert>
#include <set>

int main() {
    // PROPERTY 1: Encoded length = sum of (char_count * code_length)
    auto check_encoded_length = [](const std::string& text, 
                                    const std::unordered_map<unsigned char, std::string>& codes,
                                    const std::string& encoded) {
        size_t expected_length = 0;
        for (unsigned char ch : text) {
            expected_length += codes.at(ch).length();
        }
        assert(encoded.length() == expected_length && "Encoded length must match sum of code lengths");
    };
    
    // PROPERTY 2: Encoded string contains only '0' and '1'
    auto check_binary_string = [](const std::string& encoded) {
        for (char c : encoded) {
            assert((c == '0' || c == '1') && "Encoded string must contain only '0' and '1'");
        }
    };
    
    // PROPERTY 3: Empty text -> empty encoding
    auto check_empty_invariant = [](const std::string& text, const std::string& encoded) {
        if (text.empty()) {
            assert(encoded.empty() && "Empty text must produce empty encoding");
        }
    };
    
    // PROPERTY 4: Encoding is deterministic (same input -> same output)
    auto check_deterministic = [](const std::string& text,
                                   const std::unordered_map<unsigned char, std::string>& codes) {
        std::string encoded1 = Huffman::encode_text(text, codes);
        std::string encoded2 = Huffman::encode_text(text, codes);
        assert(encoded1 == encoded2 && "Encoding must be deterministic");
    };
    
    // Test 1: Simple encoding
    std::unordered_map<unsigned char, std::string> codes1 = {
        {'a', "0"}, {'b', "10"}, {'c', "11"}
    };
    std::string text1 = "abc";
    std::string encoded1 = Huffman::encode_text(text1, codes1);
    assert(encoded1 == "01011" && "Encoded text should be '01011'");
    check_encoded_length(text1, codes1, encoded1);
    check_binary_string(encoded1);
    check_deterministic(text1, codes1);
    
    // Test 2: Repeated characters
    std::unordered_map<unsigned char, std::string> codes2 = {
        {'x', "0"}, {'y', "1"}
    };
    std::string text2 = "xyx";
    std::string encoded2 = Huffman::encode_text(text2, codes2);
    assert(encoded2 == "010" && "Encoded text should be '010'");
    check_encoded_length(text2, codes2, encoded2);
    check_binary_string(encoded2);
    check_deterministic(text2, codes2);
    
    // Test 3: Empty text
    std::string text3 = "";
    std::string encoded3 = Huffman::encode_text(text3, codes1);
    assert(encoded3.empty() && "Empty text should produce empty encoding");
    check_empty_invariant(text3, encoded3);
    check_binary_string(encoded3);
    
    // Test 4: Single character repeated
    std::unordered_map<unsigned char, std::string> codes4 = {{'z', "111"}};
    std::string text4 = "zzzz";
    std::string encoded4 = Huffman::encode_text(text4, codes4);
    assert(encoded4 == "111111111111" && "Four 'z' chars should produce twelve '1's");
    assert(encoded4.length() == 12 && "Length should be 4 * 3 = 12");
    check_encoded_length(text4, codes4, encoded4);
    check_binary_string(encoded4);
    
    // Test 5: Variable length codes
    std::unordered_map<unsigned char, std::string> codes5 = {
        {'a', "0"}, {'b', "10"}, {'c', "110"}, {'d', "111"}
    };
    std::string text5 = "abcd";
    std::string encoded5 = Huffman::encode_text(text5, codes5);
    assert(encoded5 == "010110111" && "Encoded text should be '010110111'");
    assert(encoded5.length() == 9 && "Length should be 1+2+3+3 = 9");
    check_encoded_length(text5, codes5, encoded5);
    check_binary_string(encoded5);
    
    // Test 6: Binary/non-ASCII characters
    std::unordered_map<unsigned char, std::string> codes6 = {
        {(unsigned char)0, "00"}, {(unsigned char)255, "01"}, {(unsigned char)128, "1"}
    };
    std::string text6;
    text6.push_back('\x00');
    text6.push_back('\xFF');
    text6.push_back('\x80');
    std::string encoded6 = Huffman::encode_text(text6, codes6);
    assert(encoded6 == "00011" && "Binary characters should encode correctly");
    check_encoded_length(text6, codes6, encoded6);
    check_binary_string(encoded6);
    
    // Test 7: Long text with pattern
    std::unordered_map<unsigned char, std::string> codes7 = {{'m', "0"}, {'n', "1"}};
    std::string text7(1000, 'm');
    text7 += std::string(500, 'n');
    std::string encoded7 = Huffman::encode_text(text7, codes7);
    assert(encoded7.length() == 1500 && "1000 'm' + 500 'n' should produce 1500 bits");
    check_encoded_length(text7, codes7, encoded7);
    check_binary_string(encoded7);
    
    // Verify pattern: first 1000 should be '0', next 500 should be '1'
    for (size_t i = 0; i < 1000; ++i) {
        assert(encoded7[i] == '0' && "First 1000 bits should be '0'");
    }
    for (size_t i = 1000; i < 1500; ++i) {
        assert(encoded7[i] == '1' && "Next 500 bits should be '1'");
    }
    
    // Test 8: All characters in text must have codes
    std::unordered_map<unsigned char, std::string> codes8 = {{'p', "0"}, {'q', "1"}};
    std::string text8 = "pqpqp";
    std::string encoded8 = Huffman::encode_text(text8, codes8);
    assert(encoded8 == "01010" && "Encoded text should be '01010'");
    
    // Verify every character in text has a code
    std::set<unsigned char> text_chars(text8.begin(), text8.end());
    for (unsigned char ch : text_chars) {
        assert(codes8.count(ch) > 0 && "All text characters must have codes");
    }
    
    // Test 9: Special characters and whitespace
    std::unordered_map<unsigned char, std::string> codes9 = {
        {'\n', "00"}, {'\t', "01"}, {' ', "10"}, {'!', "11"}
    };
    std::string text9 = "\n\t !";
    std::string encoded9 = Huffman::encode_text(text9, codes9);
    assert(encoded9 == "00011011" && "Special characters should encode correctly");
    check_encoded_length(text9, codes9, encoded9);
    check_binary_string(encoded9);
    
    return 0;
}