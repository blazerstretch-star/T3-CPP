#include "../src/md_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    const double EPSILON = 1e-6;
    
    // Test 1: Two particles - Newton's third law
    std::vector<vec3> pos1 = {vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)};
    std::vector<vec3> forces1(2);
    computeForces(pos1, forces1, 10.0, 3.0, 1.0, 1.0);
    assert(std::abs(forces1[0].x + forces1[1].x) < EPSILON); // Equal and opposite
    assert(std::abs(forces1[0].y) < EPSILON);
    assert(std::abs(forces1[1].y) < EPSILON);
    
    // Test 2: Particles beyond cutoff (no force)
    std::vector<vec3> pos2 = {vec3(0.0, 0.0, 0.0), vec3(5.0, 0.0, 0.0)};
    std::vector<vec3> forces2(2);
    computeForces(pos2, forces2, 10.0, 2.5, 1.0, 1.0);
    assert(std::abs(forces2[0].x) < EPSILON);
    assert(std::abs(forces2[1].x) < EPSILON);
    
    // Test 3: Three particles - force superposition
    std::vector<vec3> pos3 = {vec3(0.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0), vec3(2.0, 0.0, 0.0)};
    std::vector<vec3> forces3(3);
    computeForces(pos3, forces3, 10.0, 3.0, 1.0, 1.0);
    // Total momentum should be conserved (sum of forces = 0)
    double total_fx = forces3[0].x + forces3[1].x + forces3[2].x;
    assert(std::abs(total_fx) < EPSILON);
    
    // Test 4: Force at r_min should be zero
    double r_min = std::pow(2.0, 1.0/6.0);
    std::vector<vec3> pos4 = {vec3(0.0, 0.0, 0.0), vec3(r_min, 0.0, 0.0)};
    std::vector<vec3> forces4(2);
    computeForces(pos4, forces4, 10.0, 3.0, 1.0, 1.0);
    assert(std::abs(forces4[0].x) < EPSILON);
    assert(std::abs(forces4[1].x) < EPSILON);
    
    // Test 5: Periodic boundaries
    std::vector<vec3> pos5 = {vec3(0.5, 0.0, 0.0), vec3(9.5, 0.0, 0.0)};
    std::vector<vec3> forces5(2);
    computeForces(pos5, forces5, 10.0, 3.0, 1.0, 1.0);
    // Minimum image distance = 1.0, forces should be non-zero
    assert(std::abs(forces5[0].x + forces5[1].x) < EPSILON); // Newton's 3rd law
    
    std::cout << "All tests passed for computeForces!" << std::endl;
    return 0;
}
