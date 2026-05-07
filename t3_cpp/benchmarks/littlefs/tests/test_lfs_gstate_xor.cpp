#include "../src/lfs_functions.h"
#include <cassert>
#include <iostream>

int main() {
    // ========================================
    // PROPERTY 1: Involution - XOR twice returns to original
    // ========================================
    auto check_involution = [](lfs_gstate_t a, const lfs_gstate_t& b) {
        lfs_gstate_t original = a;
        lfs_gstate_xor(&a, &b);
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == original.tag && 
               a.pair[0] == original.pair[0] && 
               a.pair[1] == original.pair[1] && 
               "Property: XOR(XOR(a,b),b) == a (involution)");
    };
    
    // ========================================
    // PROPERTY 2: Identity - XOR with zero state is identity
    // ========================================
    auto check_identity = [](lfs_gstate_t a) {
        lfs_gstate_t original = a;
        lfs_gstate_t zero = {0, {0, 0}};
        lfs_gstate_xor(&a, &zero);
        
        assert(a.tag == original.tag && 
               a.pair[0] == original.pair[0] && 
               a.pair[1] == original.pair[1] && 
               "Property: XOR with zero is identity");
    };
    
    // ========================================
    // PROPERTY 3: Self-inverse - XOR with itself gives zero
    // ========================================
    auto check_self_inverse = [](const lfs_gstate_t& a) {
        lfs_gstate_t result = a;
        lfs_gstate_xor(&result, &a);
        
        assert(result.tag == 0 && 
               result.pair[0] == 0 && 
               result.pair[1] == 0 && 
               "Property: XOR(a,a) == 0 (self-inverse)");
    };
    
    // ========================================
    // PROPERTY 4: Commutativity - XOR(a,b) == XOR(b,a)
    // ========================================
    auto check_commutativity = [](const lfs_gstate_t& a, const lfs_gstate_t& b) {
        lfs_gstate_t result1 = a;
        lfs_gstate_xor(&result1, &b);
        
        lfs_gstate_t result2 = b;
        lfs_gstate_xor(&result2, &a);
        
        assert(result1.tag == result2.tag && 
               result1.pair[0] == result2.pair[0] && 
               result1.pair[1] == result2.pair[1] && 
               "Property: XOR is commutative");
    };
    
    // ========================================
    // PROPERTY 5: Associativity - XOR(XOR(a,b),c) == XOR(a,XOR(b,c))
    // ========================================
    auto check_associativity = [](const lfs_gstate_t& a, const lfs_gstate_t& b, 
                                    const lfs_gstate_t& c) {
        // (a XOR b) XOR c
        lfs_gstate_t result1 = a;
        lfs_gstate_xor(&result1, &b);
        lfs_gstate_xor(&result1, &c);
        
        // a XOR (b XOR c)
        lfs_gstate_t temp = b;
        lfs_gstate_xor(&temp, &c);
        lfs_gstate_t result2 = a;
        lfs_gstate_xor(&result2, &temp);
        
        assert(result1.tag == result2.tag && 
               result1.pair[0] == result2.pair[0] && 
               result1.pair[1] == result2.pair[1] && 
               "Property: XOR is associative");
    };
    
    // ========================================
    // PROPERTY 6: Each field XORed independently
    // ========================================
    auto check_field_independence = [](const lfs_gstate_t& a, const lfs_gstate_t& b) {
        lfs_gstate_t result = a;
        lfs_gstate_xor(&result, &b);
        
        assert(result.tag == (a.tag ^ b.tag) && 
               "Property: tag field XORed independently");
        assert(result.pair[0] == (a.pair[0] ^ b.pair[0]) && 
               "Property: pair[0] field XORed independently");
        assert(result.pair[1] == (a.pair[1] ^ b.pair[1]) && 
               "Property: pair[1] field XORed independently");
    };
    
    // ========================================
    // TEST CASES
    // ========================================
    
    // Test 1: Basic XOR operation
    {
        lfs_gstate_t a = {0x12345678, {10, 20}};
        lfs_gstate_t b = {0x11111111, {5, 15}};
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == (0x12345678 ^ 0x11111111));
        assert(a.pair[0] == (10 ^ 5));
        assert(a.pair[1] == (20 ^ 15));
    }
    
    // Test 2: XOR with zero (identity)
    {
        lfs_gstate_t a = {0xFFFFFFFF, {100, 200}};
        lfs_gstate_t zero = {0x00000000, {0, 0}};
        lfs_gstate_xor(&a, &zero);
        
        assert(a.tag == 0xFFFFFFFF);
        assert(a.pair[0] == 100);
        assert(a.pair[1] == 200);
        
        check_identity({0x12345678, {50, 75}});
    }
    
    // Test 3: Self-inverse (XOR with itself)
    {
        lfs_gstate_t a = {0xABCDEF12, {123, 456}};
        lfs_gstate_t b = a;
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == 0);
        assert(a.pair[0] == 0);
        assert(a.pair[1] == 0);
        
        check_self_inverse({0x12345678, {10, 20}});
    }
    
    // Test 4: Involution (double XOR)
    {
        lfs_gstate_t a = {0x12345678, {10, 20}};
        lfs_gstate_t b = {0xABCDEF12, {30, 40}};
        
        check_involution(a, b);
        check_involution({0xFFFFFFFF, {100, 200}}, {0x11111111, {50, 75}});
    }
    
    // Test 5: Commutativity
    {
        lfs_gstate_t a = {0x12345678, {10, 20}};
        lfs_gstate_t b = {0xABCDEF12, {30, 40}};
        
        check_commutativity(a, b);
        check_commutativity({0xFFFFFFFF, {1, 2}}, {0x00000000, {3, 4}});
    }
    
    // Test 6: Associativity
    {
        lfs_gstate_t a = {0x11111111, {10, 20}};
        lfs_gstate_t b = {0x22222222, {30, 40}};
        lfs_gstate_t c = {0x44444444, {50, 60}};
        
        check_associativity(a, b, c);
    }
    
    // Test 7: Field independence
    {
        lfs_gstate_t a = {0x12345678, {100, 200}};
        lfs_gstate_t b = {0xABCDEF12, {50, 75}};
        
        check_field_independence(a, b);
    }
    
    // Test 8: All zeros
    {
        lfs_gstate_t a = {0, {0, 0}};
        lfs_gstate_t b = {0, {0, 0}};
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == 0 && a.pair[0] == 0 && a.pair[1] == 0);
    }
    
    // Test 9: All ones
    {
        lfs_gstate_t a = {0xFFFFFFFF, {0xFFFFFFFF, 0xFFFFFFFF}};
        lfs_gstate_t b = {0xFFFFFFFF, {0xFFFFFFFF, 0xFFFFFFFF}};
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == 0 && a.pair[0] == 0 && a.pair[1] == 0);
    }
    
    // Test 10: Alternating bits
    {
        lfs_gstate_t a = {0xAAAAAAAA, {0xAAAAAAAA, 0xAAAAAAAA}};
        lfs_gstate_t b = {0x55555555, {0x55555555, 0x55555555}};
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == 0xFFFFFFFF);
        assert(a.pair[0] == 0xFFFFFFFF);
        assert(a.pair[1] == 0xFFFFFFFF);
    }
    
    // Test 11: Single bit differences
    {
        lfs_gstate_t a = {0x00000000, {0, 0}};
        lfs_gstate_t b = {0x00000001, {1, 1}};
        lfs_gstate_xor(&a, &b);
        
        assert(a.tag == 0x00000001);
        assert(a.pair[0] == 1);
        assert(a.pair[1] == 1);
    }
    
    // Test 12: Chain of XOR operations
    {
        lfs_gstate_t result = {0, {0, 0}};
        lfs_gstate_t states[] = {
            {0x11111111, {10, 20}},
            {0x22222222, {30, 40}},
            {0x44444444, {50, 60}},
            {0x88888888, {70, 80}}
        };
        
        // XOR all states
        for (const auto& state : states) {
            lfs_gstate_xor(&result, &state);
        }
        
        // XOR all states again (should return to zero)
        for (const auto& state : states) {
            lfs_gstate_xor(&result, &state);
        }
        
        assert(result.tag == 0 && result.pair[0] == 0 && result.pair[1] == 0);
    }
    
    // Test 13: Stress test - verify all properties
    {
        lfs_gstate_t test_states[] = {
            {0x00000000, {0, 0}},
            {0xFFFFFFFF, {0xFFFFFFFF, 0xFFFFFFFF}},
            {0x12345678, {100, 200}},
            {0xABCDEF12, {50, 75}},
            {0x55555555, {0xAAAAAAAA, 0x55555555}},
            {0xAAAAAAAA, {0x55555555, 0xAAAAAAAA}}
        };
        
        for (const auto& a : test_states) {
            for (const auto& b : test_states) {
                check_involution(a, b);
                check_commutativity(a, b);
                check_field_independence(a, b);
                
                for (const auto& c : test_states) {
                    check_associativity(a, b, c);
                }
            }
            check_identity(a);
            check_self_inverse(a);
        }
    }
    
    // Test 14: Delta encoding use case
    {
        // Simulate wear leveling delta encoding
        lfs_gstate_t state1 = {0x12345678, {100, 200}};
        lfs_gstate_t state2 = {0x12345679, {100, 201}};
        
        // Compute delta
        lfs_gstate_t delta = state1;
        lfs_gstate_xor(&delta, &state2);
        
        // Apply delta to state1 should give state2
        lfs_gstate_t reconstructed = state1;
        lfs_gstate_xor(&reconstructed, &delta);
        
        assert(reconstructed.tag == state2.tag);
        assert(reconstructed.pair[0] == state2.pair[0]);
        assert(reconstructed.pair[1] == state2.pair[1]);
    }
    
    // Test 15: Verify modification in-place
    {
        lfs_gstate_t a = {0x12345678, {10, 20}};
        lfs_gstate_t* ptr = &a;
        lfs_gstate_t b = {0x11111111, {5, 15}};
        
        lfs_gstate_xor(ptr, &b);
        
        // Verify 'a' was modified
        assert(a.tag == (0x12345678 ^ 0x11111111));
        assert(a.pair[0] == (10 ^ 5));
        assert(a.pair[1] == (20 ^ 15));
    }
    
    std::cout << "All lfs_gstate_xor tests passed!" << std::endl;
    return 0;
}
