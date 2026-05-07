#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-10;
    
    // Test 1: 8 particles (2x2x2 lattice)
    std::vector<vec3> pos1(8);
    double boxSize1 = 10.0;
    initializePositionsLattice(pos1, boxSize1);
    
    // Check all particles are within box
    for (const auto& p : pos1) {
        assert(p.x >= 0.0 && p.x < boxSize1);
        assert(p.y >= 0.0 && p.y < boxSize1);
        assert(p.z >= 0.0 && p.z < boxSize1);
    }
    
    // Check first particle is at origin
    assert(std::abs(pos1[0].x - 0.0) < EPSILON);
    assert(std::abs(pos1[0].y - 0.0) < EPSILON);
    assert(std::abs(pos1[0].z - 0.0) < EPSILON);
    
    // Test 2: Single particle
    std::vector<vec3> pos2(1);
    double boxSize2 = 5.0;
    initializePositionsLattice(pos2, boxSize2);
    assert(std::abs(pos2[0].x - 0.0) < EPSILON);
    assert(std::abs(pos2[0].y - 0.0) < EPSILON);
    assert(std::abs(pos2[0].z - 0.0) < EPSILON);
    
    // Test 3: 27 particles (3x3x3 lattice)
    std::vector<vec3> pos3(27);
    double boxSize3 = 9.0;
    initializePositionsLattice(pos3, boxSize3);
    
    // Check spacing is uniform (lattice fills z, then y, then x)
    double spacing = boxSize3 / 3.0;
    // pos3[0] = (0,0,0), pos3[1] = (0,0,spacing), pos3[9] = (0,spacing,0)
    assert(std::abs(pos3[1].z - spacing) < EPSILON);
    
    // Test 4: Check no overlapping positions
    std::vector<vec3> pos4(10);
    double boxSize4 = 10.0;
    initializePositionsLattice(pos4, boxSize4);
    
    for (size_t i = 0; i < pos4.size(); ++i) {
        for (size_t j = i + 1; j < pos4.size(); ++j) {
            vec3 dr = pos4[i] - pos4[j];
            double dist = dr.length();
            assert(dist > EPSILON); // No two particles at same position
        }
    }
    
    std::cout << "All tests passed for initializePositionsLattice!" << std::endl;
    return 0;
}
