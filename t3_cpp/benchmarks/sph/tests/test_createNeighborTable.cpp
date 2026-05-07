#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>

int main() {
    const size_t count = 10;
    Particle particles[count];
    
    // Test 1: Assign specific hash values (sorted)
    particles[0].hash = 100;
    particles[1].hash = 100;
    particles[2].hash = 200;
    particles[3].hash = 200;
    particles[4].hash = 200;
    particles[5].hash = 300;
    particles[6].hash = 400;
    particles[7].hash = 400;
    particles[8].hash = 500;
    particles[9].hash = 600;
    
    uint32_t* table = createNeighborTable(particles, count);
    
    // Test 2: Verify first occurrence indices
    assert(table[100] == 0 && "Hash 100 must start at index 0");
    assert(table[200] == 2 && "Hash 200 must start at index 2");
    assert(table[300] == 5 && "Hash 300 must start at index 5");
    assert(table[400] == 6 && "Hash 400 must start at index 6");
    assert(table[500] == 8 && "Hash 500 must start at index 8");
    assert(table[600] == 9 && "Hash 600 must start at index 9");
    
    // Test 3: Unused hashes must be NO_PARTICLE
    assert(table[0] == NO_PARTICLE && "Unused hash 0 must be NO_PARTICLE");
    assert(table[1] == NO_PARTICLE && "Unused hash 1 must be NO_PARTICLE");
    assert(table[99] == NO_PARTICLE && "Unused hash 99 must be NO_PARTICLE");
    assert(table[150] == NO_PARTICLE && "Unused hash 150 must be NO_PARTICLE");
    assert(table[250] == NO_PARTICLE && "Unused hash 250 must be NO_PARTICLE");
    assert(table[TABLE_SIZE - 1] == NO_PARTICLE && "Last entry must be NO_PARTICLE");
    
    // Test 4: Single particle per hash
    Particle particles2[5];
    particles2[0].hash = 10;
    particles2[1].hash = 20;
    particles2[2].hash = 30;
    particles2[3].hash = 40;
    particles2[4].hash = 50;
    uint32_t* table2 = createNeighborTable(particles2, 5);
    assert(table2[10] == 0 && "Single particle hash 10");
    assert(table2[20] == 1 && "Single particle hash 20");
    assert(table2[30] == 2 && "Single particle hash 30");
    assert(table2[40] == 3 && "Single particle hash 40");
    assert(table2[50] == 4 && "Single particle hash 50");
    free(table2);
    
    // Test 5: All particles same hash
    Particle particles3[4];
    particles3[0].hash = 777;
    particles3[1].hash = 777;
    particles3[2].hash = 777;
    particles3[3].hash = 777;
    uint32_t* table3 = createNeighborTable(particles3, 4);
    assert(table3[777] == 0 && "All same hash must point to first");
    assert(table3[776] == NO_PARTICLE && "Adjacent hash must be NO_PARTICLE");
    assert(table3[778] == NO_PARTICLE && "Adjacent hash must be NO_PARTICLE");
    free(table3);
    
    // Test 6: Empty particle array
    uint32_t* table4 = createNeighborTable(particles, 0);
    assert(table4[0] == NO_PARTICLE && "Empty array: all NO_PARTICLE");
    assert(table4[100] == NO_PARTICLE && "Empty array: all NO_PARTICLE");
    free(table4);
    
    // Test 7: Verify all table entries are either NO_PARTICLE or a valid particle index
    for (size_t i = 0; i < TABLE_SIZE; i++) {
        assert((table[i] == NO_PARTICLE || table[i] < count) && "Invalid index in table");
    }

    // Test 8: All entries between hash groups must be NO_PARTICLE
    for (uint32_t i = 101; i < 200; i++)
        assert(table[i] == NO_PARTICLE && "Gap between hash 100 and 200 must be NO_PARTICLE");
    for (uint32_t i = 201; i < 300; i++)
        assert(table[i] == NO_PARTICLE && "Gap between hash 200 and 300 must be NO_PARTICLE");
    for (uint32_t i = 301; i < 400; i++)
        assert(table[i] == NO_PARTICLE && "Gap between hash 300 and 400 must be NO_PARTICLE");

    free(table);
    std::cout << "All createNeighborTable tests passed!" << std::endl;
    return 0;
}
