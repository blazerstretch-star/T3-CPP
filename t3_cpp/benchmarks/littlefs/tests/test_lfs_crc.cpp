#include "../src/lfs_functions.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

int main() {
    // ========================================
    // PROPERTY 1: Deterministic - same input produces same output
    // ========================================
    auto check_deterministic = [](uint32_t init_crc, const void* data, size_t size) {
        uint32_t crc1 = lfs_crc(init_crc, data, size);
        uint32_t crc2 = lfs_crc(init_crc, data, size);
        assert(crc1 == crc2 && 
               "Property: CRC must be deterministic");
    };
    
    // ========================================
    // PROPERTY 2: Different data produces different CRC (high probability)
    // ========================================
    auto check_uniqueness = [](const char* data1, const char* data2) {
        if (strcmp(data1, data2) == 0) return; // Skip identical data
        uint32_t crc1 = lfs_crc(0xFFFFFFFF, data1, strlen(data1));
        uint32_t crc2 = lfs_crc(0xFFFFFFFF, data2, strlen(data2));
        // Different data should produce different CRC (not guaranteed but highly likely)
        assert(crc1 != crc2 && 
               "Property: Different data should produce different CRC");
    };
    
    // ========================================
    // PROPERTY 3: Empty data with same init produces same result
    // ========================================
    auto check_empty_consistency = [](uint32_t init_crc) {
        uint32_t result = lfs_crc(init_crc, "", 0);
        assert(result == init_crc && 
               "Property: CRC of empty data should return init value");
    };
    
    // ========================================
    // PROPERTY 4: Single byte change produces different CRC
    // ========================================
    auto check_avalanche = [](const char* data, size_t len) {
        if (len == 0) return;
        std::vector<char> modified(data, data + len);
        uint32_t original_crc = lfs_crc(0xFFFFFFFF, data, len);
        
        // Flip one bit in the middle
        modified[len / 2] ^= 0x01;
        uint32_t modified_crc = lfs_crc(0xFFFFFFFF, modified.data(), len);
        
        assert(original_crc != modified_crc && 
               "Property: Single bit change should alter CRC (avalanche effect)");
    };
    
    // ========================================
    // PROPERTY 5: Order matters - CRC(AB) != CRC(BA)
    // ========================================
    auto check_order_sensitivity = [](const char* a, const char* b) {
        std::string ab = std::string(a) + b;
        std::string ba = std::string(b) + a;
        if (ab == ba) return; // Skip if palindrome
        
        uint32_t crc_ab = lfs_crc(0xFFFFFFFF, ab.c_str(), ab.length());
        uint32_t crc_ba = lfs_crc(0xFFFFFFFF, ba.c_str(), ba.length());
        
        assert(crc_ab != crc_ba && 
               "Property: Order of data should affect CRC");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Known test vectors (from original implementation)
    {
        const char *data1 = "hello";
        uint32_t crc1 = lfs_crc(0xFFFFFFFF, data1, strlen(data1));
        assert(crc1 == 0xc9ef5979 && "Known vector: 'hello'");
        
        const char *data2 = "world";
        uint32_t crc2 = lfs_crc(0xFFFFFFFF, data2, strlen(data2));
        assert(crc2 == 0xc588eebc && "Known vector: 'world'");
        
        uint32_t crc3 = lfs_crc(0x00000000, "test", 4);
        assert(crc3 == 0xf93ba110 && "Known vector: 'test'");
    }
    
    // Test 2: Empty data
    {
        uint32_t crc_empty1 = lfs_crc(0xFFFFFFFF, "", 0);
        assert(crc_empty1 == 0xFFFFFFFF);
        
        uint32_t crc_empty2 = lfs_crc(0x00000000, "", 0);
        assert(crc_empty2 == 0x00000000);
        
        check_empty_consistency(0xFFFFFFFF);
        check_empty_consistency(0x12345678);
    }
    
    // Test 3: Single byte
    {
        uint8_t byte_a = 'A';
        uint32_t crc_a = lfs_crc(0xFFFFFFFF, &byte_a, 1);
        
        uint8_t byte_b = 'B';
        uint32_t crc_b = lfs_crc(0xFFFFFFFF, &byte_b, 1);
        
        assert(crc_a != crc_b && "Different bytes produce different CRC");
        check_deterministic(0xFFFFFFFF, &byte_a, 1);
    }
    
    // Test 4: Determinism
    {
        const char* test_data = "filesystem integrity check";
        check_deterministic(0xFFFFFFFF, test_data, strlen(test_data));
        check_deterministic(0x00000000, test_data, strlen(test_data));
        check_deterministic(0x12345678, test_data, strlen(test_data));
    }
    
    // Test 5: Different data produces different CRC
    {
        check_uniqueness("hello", "world");
        check_uniqueness("test", "data");
        check_uniqueness("a", "b");
        check_uniqueness("short", "longer string");
    }
    
    // Test 6: Binary data (null bytes)
    {
        uint8_t binary1[] = {0x00, 0x01, 0x02, 0x03};
        uint8_t binary2[] = {0x00, 0x01, 0x02, 0x04};
        
        uint32_t crc_bin1 = lfs_crc(0xFFFFFFFF, binary1, 4);
        uint32_t crc_bin2 = lfs_crc(0xFFFFFFFF, binary2, 4);
        
        assert(crc_bin1 != crc_bin2 && "Binary data: different bytes produce different CRC");
        check_deterministic(0xFFFFFFFF, binary1, 4);
    }
    
    // Test 7: Avalanche effect - single bit change
    {
        const char* data = "The quick brown fox jumps over the lazy dog";
        check_avalanche(data, strlen(data));
        
        const char* short_data = "test";
        check_avalanche(short_data, strlen(short_data));
    }
    
    // Test 8: Order sensitivity
    {
        check_order_sensitivity("hello", "world");
        check_order_sensitivity("ab", "cd");
        check_order_sensitivity("test", "data");
    }
    
    // Test 9: Large data
    {
        std::vector<uint8_t> large_data(10000);
        for (size_t i = 0; i < large_data.size(); i++) {
            large_data[i] = i % 256;
        }
        
        uint32_t crc_large = lfs_crc(0xFFFFFFFF, large_data.data(), large_data.size());
        check_deterministic(0xFFFFFFFF, large_data.data(), large_data.size());
        
        // Modify one byte
        large_data[5000] ^= 0x01;
        uint32_t crc_modified = lfs_crc(0xFFFFFFFF, large_data.data(), large_data.size());
        assert(crc_large != crc_modified && "Large data: single byte change detected");
    }
    
    // Test 10: Different initial CRC values
    {
        const char* data = "test data";
        uint32_t crc1 = lfs_crc(0xFFFFFFFF, data, strlen(data));
        uint32_t crc2 = lfs_crc(0x00000000, data, strlen(data));
        uint32_t crc3 = lfs_crc(0x12345678, data, strlen(data));
        
        // Different init values should produce different results
        assert(crc1 != crc2 && "Different init CRC produces different result");
        assert(crc2 != crc3 && "Different init CRC produces different result");
        assert(crc1 != crc3 && "Different init CRC produces different result");
    }
    
    // Test 11: All zero bytes
    {
        uint8_t zeros[100] = {0};
        uint32_t crc_zeros = lfs_crc(0xFFFFFFFF, zeros, 100);
        check_deterministic(0xFFFFFFFF, zeros, 100);
        
        // Should differ from all 0xFF bytes
        uint8_t ones[100];
        memset(ones, 0xFF, 100);
        uint32_t crc_ones = lfs_crc(0xFFFFFFFF, ones, 100);
        assert(crc_zeros != crc_ones && "All zeros != all ones");
    }
    
    // Test 12: Incremental CRC (chaining)
    {
        const char* part1 = "hello";
        const char* part2 = "world";
        std::string combined = std::string(part1) + part2;
        
        // Single pass
        uint32_t crc_single = lfs_crc(0xFFFFFFFF, combined.c_str(), combined.length());
        
        // Two passes (chained)
        uint32_t crc_part1 = lfs_crc(0xFFFFFFFF, part1, strlen(part1));
        uint32_t crc_chained = lfs_crc(crc_part1, part2, strlen(part2));
        
        assert(crc_single == crc_chained && 
               "Property: CRC can be computed incrementally");
    }
    
    std::cout << "All lfs_crc tests passed!" << std::endl;
    return 0;
}
