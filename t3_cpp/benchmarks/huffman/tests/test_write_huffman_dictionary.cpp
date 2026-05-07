#include "../src/huffman_functions.h"
#include <cassert>
#include <sstream>
#include <cstdio>

int main() {
    // PROPERTY 1: File size = 1 (dict_size) + sum(1 + 1 + code_length) for each entry
    auto check_file_size = [](const std::string& filename, 
                              const std::unordered_map<unsigned char, std::string>& codes) {
        std::ifstream infile(filename, std::ios::binary | std::ios::ate);
        size_t file_size = infile.tellg();
        infile.close();
        
        size_t expected_size = 1; // dict_size byte
        for (const auto& entry : codes) {
            expected_size += 1; // character
            expected_size += 1; // length
            expected_size += entry.second.length(); // code string
        }
        assert(file_size == expected_size && "File size must match expected format");
    };
    
    // PROPERTY 2: Round-trip consistency (write then read)
    auto check_round_trip = [](const std::unordered_map<unsigned char, std::string>& codes) {
        std::string filename = "test_roundtrip.bin";
        
        // Write
        std::ofstream outfile(filename, std::ios::binary);
        Huffman::write_huffman_dictionary(outfile, codes);
        outfile.close();
        
        // Read back
        std::ifstream infile(filename, std::ios::binary);
        char dict_size;
        infile.read(&dict_size, sizeof(char));
        
        std::unordered_map<unsigned char, std::string> read_codes;
        for (int i = 0; i < dict_size; ++i) {
            char character, length;
            infile.read(&character, sizeof(char));
            infile.read(&length, sizeof(char));
            std::string code;
            code.resize(length);
            infile.read(&code[0], length);
            read_codes[static_cast<unsigned char>(character)] = code;
        }
        infile.close();
        
        // Verify equality
        assert(read_codes.size() == codes.size() && "Read codes size must match written codes");
        for (const auto& entry : codes) {
            assert(read_codes.count(entry.first) > 0 && "All written characters must be read back");
            assert(read_codes[entry.first] == entry.second && "Code values must match exactly");
        }
        
        std::remove(filename.c_str());
    };
    
    // Test 1: Write simple dictionary
    std::unordered_map<unsigned char, std::string> codes = {
        {'a', "0"}, {'b', "10"}, {'c', "11"}
    };
    
    std::ofstream outfile("test_dict.bin", std::ios::binary);
    Huffman::write_huffman_dictionary(outfile, codes);
    outfile.close();
    
    check_file_size("test_dict.bin", codes);
    
    // Read back and verify
    std::ifstream infile("test_dict.bin", std::ios::binary);
    char dict_size;
    infile.read(&dict_size, sizeof(char));
    assert(dict_size == 3 && "Dictionary size should be 3");
    
    // Verify entries exist (order may vary due to unordered_map)
    std::unordered_map<unsigned char, std::string> read_codes;
    for (int i = 0; i < dict_size; ++i) {
        char character, length;
        infile.read(&character, sizeof(char));
        infile.read(&length, sizeof(char));
        std::string code;
        code.resize(length);
        infile.read(&code[0], length);
        read_codes[static_cast<unsigned char>(character)] = code;
    }
    infile.close();
    
    assert(read_codes.size() == 3 && "Should read 3 codes");
    assert(read_codes['a'] == "0" && "Code for 'a' should be '0'");
    assert(read_codes['b'] == "10" && "Code for 'b' should be '10'");
    assert(read_codes['c'] == "11" && "Code for 'c' should be '11'");
    
    std::remove("test_dict.bin");
    
    // Test 2: Empty dictionary
    std::unordered_map<unsigned char, std::string> codes2;
    check_round_trip(codes2);
    
    // Test 3: Single entry
    std::unordered_map<unsigned char, std::string> codes3 = {{'x', ""}};
    check_round_trip(codes3);
    
    // Test 4: Variable length codes
    std::unordered_map<unsigned char, std::string> codes4 = {
        {'a', "0"}, {'b', "10"}, {'c', "110"}, {'d', "1110"}, {'e', "11110"}
    };
    check_round_trip(codes4);
    
    // Test 5: Binary/non-ASCII characters
    std::unordered_map<unsigned char, std::string> codes5 = {
        {0, "00"}, {255, "01"}, {128, "10"}, {64, "11"}
    };
    check_round_trip(codes5);
    
    // Test 6: Long codes
    std::unordered_map<unsigned char, std::string> codes6 = {
        {'a', "0000000000"}, {'b', "1111111111"}
    };
    check_round_trip(codes6);
    
    // Test 7: Many entries (stress test)
    std::unordered_map<unsigned char, std::string> codes7;
    for (int i = 0; i < 26; ++i) {
        codes7[static_cast<unsigned char>('a' + i)] = std::string(i % 5 + 1, '0' + (i % 2));
    }
    check_round_trip(codes7);
    
    // Test 8: Special characters
    std::unordered_map<unsigned char, std::string> codes8 = {
        {'\n', "00"}, {'\t', "01"}, {'\r', "10"}, {' ', "11"}
    };
    check_round_trip(codes8);
    
    // Test 9: Verify binary format integrity
    std::unordered_map<unsigned char, std::string> codes9 = {{'p', "101"}};
    std::ofstream out9("test_format.bin", std::ios::binary);
    Huffman::write_huffman_dictionary(out9, codes9);
    out9.close();
    
    // Read raw bytes and verify format
    std::ifstream in9("test_format.bin", std::ios::binary);
    char byte1, byte2, byte3;
    in9.read(&byte1, 1); // dict_size = 1
    in9.read(&byte2, 1); // character = 'p'
    in9.read(&byte3, 1); // length = 3
    
    assert(byte1 == 1 && "First byte should be dict_size = 1");
    assert(byte2 == 'p' && "Second byte should be character 'p'");
    assert(byte3 == 3 && "Third byte should be length = 3");
    
    char code_bytes[3];
    in9.read(code_bytes, 3);
    assert(code_bytes[0] == '1' && code_bytes[1] == '0' && code_bytes[2] == '1' && 
           "Code bytes should be '101'");
    
    in9.close();
    std::remove("test_format.bin");
    
    return 0;
}