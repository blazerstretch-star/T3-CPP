#include "../src/huffman_functions.h"
#include <cassert>
#include <set>
#include <numeric>

int main() {
    // PROPERTY 1: Sum of frequencies = input length
    auto check_sum_property = [](const std::string& text, const std::unordered_map<unsigned char, int>& freq) {
        int sum = 0;
        for (const auto& p : freq) sum += p.second;
        assert(sum == (int)text.length() && "Sum of frequencies must equal text length");
    };
    
    // PROPERTY 2: All counts > 0
    auto check_positive_counts = [](const std::unordered_map<unsigned char, int>& freq) {
        for (const auto& p : freq) {
            assert(p.second > 0 && "All frequency counts must be positive");
        }
    };
    
    // PROPERTY 3: Keys ⊆ input chars
    auto check_keys_subset = [](const std::string& text, const std::unordered_map<unsigned char, int>& freq) {
        std::set<unsigned char> text_chars(text.begin(), text.end());
        for (const auto& p : freq) {
            assert(text_chars.count(p.first) > 0 && "All keys must be from input text");
        }
        assert(freq.size() == text_chars.size() && "Map size must equal unique character count");
    };
    
    // Test 1: Simple text
    std::string text1 = "aab";
    auto freq1 = Huffman::build_frequency_table(text1);
    assert(freq1['a'] == 2 && "Character 'a' should appear 2 times");
    assert(freq1['b'] == 1 && "Character 'b' should appear 1 time");
    assert(freq1.size() == 2 && "Should have 2 unique characters");
    check_sum_property(text1, freq1);
    check_positive_counts(freq1);
    check_keys_subset(text1, freq1);
    
    // Test 2: Empty string
    std::string text2 = "";
    auto freq2 = Huffman::build_frequency_table(text2);
    assert(freq2.empty() && "Empty string should produce empty frequency table");
    check_sum_property(text2, freq2);
    
    // Test 3: Single character repeated
    std::string text3 = "aaaa";
    auto freq3 = Huffman::build_frequency_table(text3);
    assert(freq3['a'] == 4 && "Character 'a' should appear 4 times");
    assert(freq3.size() == 1 && "Should have 1 unique character");
    check_sum_property(text3, freq3);
    check_positive_counts(freq3);
    check_keys_subset(text3, freq3);
    
    // Test 4: Binary/non-ASCII characters
    std::string text4;
    text4.push_back('\x00');
    text4.push_back('\xFF');
    text4.push_back('\x00');
    text4.push_back('\x80');
    auto freq4 = Huffman::build_frequency_table(text4);
    assert(freq4[(unsigned char)0] == 2 && "Null byte should appear 2 times");
    assert(freq4[(unsigned char)255] == 1 && "0xFF should appear 1 time");
    assert(freq4[(unsigned char)128] == 1 && "0x80 should appear 1 time");
    check_sum_property(text4, freq4);
    check_positive_counts(freq4);
    check_keys_subset(text4, freq4);
    
    // Test 5: All unique characters
    std::string text5 = "abcdefgh";
    auto freq5 = Huffman::build_frequency_table(text5);
    assert(freq5.size() == 8 && "Should have 8 unique characters");
    for (const auto& p : freq5) {
        assert(p.second == 1 && "Each character should appear once");
    }
    check_sum_property(text5, freq5);
    check_positive_counts(freq5);
    check_keys_subset(text5, freq5);
    
    // Test 6: Large input with repeated pattern
    std::string text6(10000, 'x');
    text6 += std::string(5000, 'y');
    auto freq6 = Huffman::build_frequency_table(text6);
    assert(freq6['x'] == 10000 && "Character 'x' should appear 10000 times");
    assert(freq6['y'] == 5000 && "Character 'y' should appear 5000 times");
    check_sum_property(text6, freq6);
    check_positive_counts(freq6);
    check_keys_subset(text6, freq6);
    
    // Test 7: Special characters and whitespace
    std::string text7 = "Hello\nWorld\t!\r\n";
    auto freq7 = Huffman::build_frequency_table(text7);
    assert(freq7['\n'] == 2 && "Newline should appear 2 times");
    assert(freq7['\t'] == 1 && "Tab should appear 1 time");
    assert(freq7['\r'] == 1 && "Carriage return should appear 1 time");
    check_sum_property(text7, freq7);
    check_positive_counts(freq7);
    check_keys_subset(text7, freq7);
    
    return 0;
}