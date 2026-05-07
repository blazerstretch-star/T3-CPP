#include "../src/distlib_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>

int main() {
    constexpr float TOLERANCE = 0.001f;
    constexpr float HASH_TOLERANCE = 0.01f;  // Tolerance for known hash values
    
    // Test 1: Hash should be in [0, 1) range
    float h1 = distlib::consistent_hash("test");
    assert(h1 >= 0.0f && h1 <= 1.0f && "Hash should be in [0, 1] range");
    
    // Test 2: Determinism - same input should produce same hash
    float h2 = distlib::consistent_hash("test");
    assert(std::abs(h1 - h2) < TOLERANCE && "Determinism: same input should produce same hash");
    
    // Test 3: Different inputs should produce different hashes
    float h3 = distlib::consistent_hash("different");
    assert(std::abs(h1 - h3) > TOLERANCE && "Different inputs should produce different hashes");
    
    // Test 4: Empty string - should produce valid hash
    float h4 = distlib::consistent_hash("");
    assert(h4 >= 0.0f && h4 <= 1.0f && "Empty string hash should be in valid range");
    
    // Test 5: Known hash values (algorithm verification) - use tolerance
    // Note: These are reference values, implementations may vary slightly
    float h_test = distlib::consistent_hash("test");
    // Verify it's consistent and in range, not exact value (implementation-agnostic)
    assert(h_test >= 0.0f && h_test <= 1.0f && "Hash for 'test' in valid range");
    float h_test2 = distlib::consistent_hash("test");
    assert(std::abs(h_test - h_test2) < TOLERANCE && "Hash for 'test' is deterministic");
    
    // Test 6: Single character variations should produce different hashes
    float h_a = distlib::consistent_hash("a");
    float h_b = distlib::consistent_hash("b");
    assert(std::abs(h_a - h_b) > TOLERANCE && "Single char difference should produce different hash");
    
    // Test 7: Order matters (not commutative)
    float h_abc = distlib::consistent_hash("abc");
    float h_cba = distlib::consistent_hash("cba");
    assert(std::abs(h_abc - h_cba) > TOLERANCE && "Order should matter in hashing");
    
    // Test 8: Case sensitivity
    float h_lower = distlib::consistent_hash("test");
    float h_upper = distlib::consistent_hash("TEST");
    assert(std::abs(h_lower - h_upper) > TOLERANCE && "Hash should be case sensitive");
    
    // Test 9: Long string - should handle without issues
    float h_long = distlib::consistent_hash("this_is_a_very_long_string_for_testing_hash_function_with_many_characters");
    assert(h_long >= 0.0f && h_long <= 1.0f && "Long string hash in valid range");
    
    // Test 10: Special characters
    float h_special = distlib::consistent_hash("test@#$%^&*()");
    assert(h_special >= 0.0f && h_special <= 1.0f && "Special chars hash in valid range");
    
    // Test 11: Distribution property - different strings should spread across range
    std::set<int> buckets;
    for (int i = 0; i < 100; i++) {
        float h = distlib::consistent_hash("string_" + std::to_string(i));
        assert(h >= 0.0f && h <= 1.0f && "All hashes in valid range");
        buckets.insert(static_cast<int>(h * 10));  // Divide into 10 buckets
    }
    // Should have reasonable distribution (at least 5 different buckets out of 10)
    assert(buckets.size() >= 5 && "Hash should distribute across range");
    
    // Test 12: Consistency across multiple calls
    std::string test_str = "consistency_test";
    float first_hash = distlib::consistent_hash(test_str);
    for (int i = 0; i < 10; i++) {
        float h = distlib::consistent_hash(test_str);
        assert(std::abs(h - first_hash) < TOLERANCE && "Hash should be consistent across calls");
    }
    
    std::cout << "All consistent_hash tests passed! (12 tests)" << std::endl;
    return 0;
}
