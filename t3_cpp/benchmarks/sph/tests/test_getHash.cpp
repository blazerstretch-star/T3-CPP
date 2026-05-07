#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <set>

int main() {
    // Test 1: Deterministic - same cell produces same hash
    glm::ivec3 cell1(5, 10, 15);
    uint32_t hash1 = getHash(cell1);
    uint32_t hash2 = getHash(cell1);
    assert(hash1 == hash2 && "Same cell must produce identical hash");
    
    // Test 2: Exact hash value for cell(1,2,3) — precomputed independently as 100934
    // ((uint)(1*73856093) ^ (uint)(2*19349663) ^ (uint)(3*83492791)) % 262144 = 100934
    glm::ivec3 cell2(1, 2, 3);
    assert(getHash(cell2) == 100934u && "Hash(1,2,3) must equal 100934");
    
    // Test 3: Hash function distributes well — count distinct hashes over a grid of cells.
    // A correct spatial hash must produce at least 50% unique values over 125 distinct cells.
    {
        std::set<uint32_t> seen;
        int total = 0;
        for (int x = 0; x < 5; x++)
            for (int y = 0; y < 5; y++)
                for (int z = 0; z < 5; z++) {
                    seen.insert(getHash(glm::ivec3(x, y, z)));
                    total++;
                }
        assert(seen.size() >= (size_t)(total / 2) && "Hash must distribute: at least 50% unique over 5x5x5 grid");
    }
    
    // Test 4: All hashes within valid range
    assert(hash1 < TABLE_SIZE && "Hash exceeds TABLE_SIZE");
    assert(hash2 < TABLE_SIZE && "Hash exceeds TABLE_SIZE");
    
    // Test 5: Negative coordinates
    glm::ivec3 cell6(-5, -10, -15);
    uint32_t hash6 = getHash(cell6);
    assert(hash6 < TABLE_SIZE && "Negative cell hash exceeds TABLE_SIZE");
    assert(hash6 != hash1 && "Negative cell must differ from positive");
    
    // Test 6: Large coordinates
    glm::ivec3 cell7(1000, 2000, 3000);
    uint32_t hash7 = getHash(cell7);
    assert(hash7 < TABLE_SIZE && "Large cell hash exceeds TABLE_SIZE");
    
    // Test 7: Adjacent cells produce different hashes
    glm::ivec3 cell8(10, 10, 10);
    glm::ivec3 cell9(11, 10, 10);
    assert(getHash(cell8) != getHash(cell9) && "Adjacent cells must differ");
    
    std::cout << "All getHash tests passed!" << std::endl;
    return 0;
}
