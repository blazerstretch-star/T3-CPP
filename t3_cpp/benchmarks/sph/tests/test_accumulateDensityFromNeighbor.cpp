#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.001f;
    
    // Test 1: Particles within smoothing radius
    Particle pi, pj;
    pi.position = glm::vec3(0, 0, 0);
    pj.position = glm::vec3(0.1f, 0, 0);
    float massPoly6 = 50.0f;
    float h2 = 0.25f;
    float density = 0.0f;
    
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density);
    float dist2 = 0.01f;
    float expected = massPoly6 * std::pow(h2 - dist2, 3);
    assert(std::abs(density - expected) < EPSILON && "Density accumulation mismatch");
    assert(density > 0 && "Density must be positive");
    
    // Test 2: Particles outside smoothing radius
    pj.position = glm::vec3(10.0f, 0, 0);
    float density2 = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density2);
    assert(density2 == 0 && "Outside radius must contribute zero");
    
    // Test 3: Accumulation adds to existing density
    pj.position = glm::vec3(0.1f, 0, 0);
    float density3 = 100.0f;
    float old_density = density3;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density3);
    assert(density3 > old_density && "Must accumulate to existing density");
    assert(std::abs(density3 - (old_density + expected)) < EPSILON && "Accumulation incorrect");
    
    // Test 4: Closer particles contribute more
    // pj at (0.05,0,0): dist2=0.0025 < h2=0.25 (inside)
    // pj at (0.4,0,0):  dist2=0.16   < h2=0.25 (also inside, but farther)
    pj.position = glm::vec3(0.05f, 0, 0);
    float density_close = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density_close);
    pj.position = glm::vec3(0.4f, 0, 0);
    float density_far = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density_far);
    assert(density_close > 0 && "Close particle must contribute");
    assert(density_far > 0 && "Far-but-inside particle must also contribute");
    assert(density_close > density_far && "Closer particles must contribute more");
    
    // Test 5: At boundary (dist2 = h2)
    pj.position = glm::vec3(0.5f, 0, 0);
    float density_boundary = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density_boundary);
    assert(density_boundary == 0 && "At boundary must contribute zero");
    
    // Test 6: Different massPoly6 values
    pj.position = glm::vec3(0.1f, 0, 0);
    float density_m1 = 0.0f;
    float density_m2 = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, 50.0f, h2, density_m1);
    accumulateDensityFromNeighbor(&pi, &pj, 100.0f, h2, density_m2);
    assert(std::abs(density_m2 / density_m1 - 2.0f) < 0.01f && "Mass scaling violated");
    
    // Test 7: Different axes
    pj.position = glm::vec3(0, 0.1f, 0);
    float density_y = 0.0f;
    accumulateDensityFromNeighbor(&pi, &pj, massPoly6, h2, density_y);
    assert(std::abs(density_y - expected) < EPSILON && "Y-axis contribution mismatch");
    
    std::cout << "All accumulateDensityFromNeighbor tests passed!" << std::endl;
    return 0;
}
