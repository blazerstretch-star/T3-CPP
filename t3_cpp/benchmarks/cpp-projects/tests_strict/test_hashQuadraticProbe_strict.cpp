#include "../src/ds_algorithms.h"
#include <cassert>
#include <set>

int main() {
    // Property: Result in [0, tableSize-1]
    // Property: Different attempts give different indices
    // Property: Deterministic
    
    // Test 1: Range check
    {
        int result = Algorithms::hashQuadraticProbe(10, 7, 0);
        assert(result >= 0 && result < 7 && "Result in range");
    }
    
    // Test 2: Different attempts
    {
        int r0 = Algorithms::hashQuadraticProbe(10, 7, 0);
        int r1 = Algorithms::hashQuadraticProbe(10, 7, 1);
        int r2 = Algorithms::hashQuadraticProbe(10, 7, 2);
        assert(r0 >= 0 && r0 < 7 && "Attempt 0 in range");
        assert(r1 >= 0 && r1 < 7 && "Attempt 1 in range");
        assert(r2 >= 0 && r2 < 7 && "Attempt 2 in range");
    }
    
    // Test 3: Deterministic
    {
        int r1 = Algorithms::hashQuadraticProbe(15, 11, 3);
        int r2 = Algorithms::hashQuadraticProbe(15, 11, 3);
        assert(r1 == r2 && "Deterministic");
    }
    
    // Test 4: Coverage
    {
        std::set<int> visited;
        for (int attempt = 0; attempt < 10; attempt++) {
            int idx = Algorithms::hashQuadraticProbe(5, 10, attempt);
            visited.insert(idx);
        }
        assert(visited.size() > 1 && "Probes multiple positions");
    }
    
    // Test 5: Different keys
    {
        int r1 = Algorithms::hashQuadraticProbe(10, 7, 0);
        int r2 = Algorithms::hashQuadraticProbe(20, 7, 0);
        assert(r1 >= 0 && r1 < 7 && "Key 10 in range");
        assert(r2 >= 0 && r2 < 7 && "Key 20 in range");
    }
    
    // Test 6: Large table
    {
        int result = Algorithms::hashQuadraticProbe(100, 97, 5);
        assert(result >= 0 && result < 97 && "Large table in range");
    }
    
    // Test 7: Small table
    {
        int result = Algorithms::hashQuadraticProbe(10, 3, 1);
        assert(result >= 0 && result < 3 && "Small table in range");
    }
    
    // Test 8: Zero attempt
    {
        int result = Algorithms::hashQuadraticProbe(42, 13, 0);
        assert(result >= 0 && result < 13 && "Zero attempt in range");
    }
    
    return 0;
}
