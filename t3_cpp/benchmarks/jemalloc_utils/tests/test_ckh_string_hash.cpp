#include "../src/jemalloc_utils.h"
#include <cassert>
#include <cstring>

using namespace JemallocUtils;

int main() {
    // PROPERTY 1: Hash function produces output
    const char *key1 = "test";
    size_t hash1[2];
    ckh_string_hash(key1, hash1);
    assert(hash1[0] != 0 || hash1[1] != 0);
    
    // PROPERTY 2: Determinism - same string produces same hash
    const char *key2 = "test";
    size_t hash2[2];
    ckh_string_hash(key2, hash2);
    assert(hash1[0] == hash2[0] && "Hash must be deterministic");
    assert(hash1[1] == hash2[1] && "Hash must be deterministic");
    
    // PROPERTY 3: Different strings produce different hashes
    const char *key3 = "different";
    size_t hash3[2];
    ckh_string_hash(key3, hash3);
    assert((hash1[0] != hash3[0] || hash1[1] != hash3[1]) && 
           "Different strings should produce different hashes");
    
    // PROPERTY 4: Content-based hashing (not pointer-based)
    char str1[] = "hello";
    char str2[] = "hello";
    size_t hash4[2], hash5[2];
    ckh_string_hash(str1, hash4);
    ckh_string_hash(str2, hash5);
    assert(hash4[0] == hash5[0] && hash4[1] == hash5[1] && 
           "Same content should produce same hash regardless of pointer");
    
    // PROPERTY 5: Empty string handling
    const char *empty = "";
    size_t hash6[2];
    ckh_string_hash(empty, hash6);
    // Should produce valid hash (may be zero or non-zero, but must be consistent)
    size_t hash7[2];
    ckh_string_hash(empty, hash7);
    assert(hash6[0] == hash7[0] && hash6[1] == hash7[1] && 
           "Empty string hash must be deterministic");
    
    return 0;
}
