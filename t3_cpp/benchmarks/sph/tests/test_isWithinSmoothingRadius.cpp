#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Within radius
    glm::vec3 pos1(0, 0, 0);
    glm::vec3 pos2(0.1f, 0, 0);
    float h2 = 0.25f;
    bool within = isWithinSmoothingRadius(pos1, pos2, h2);
    assert(within == true && "0.1^2 = 0.01 < 0.25 must be within");
    
    // Test 2: Outside radius
    glm::vec3 pos3(10.0f, 0, 0);
    bool outside = isWithinSmoothingRadius(pos1, pos3, h2);
    assert(outside == false && "10^2 = 100 > 0.25 must be outside");
    
    // Test 3: At exact boundary
    glm::vec3 pos4(0.5f, 0, 0);
    bool boundary = isWithinSmoothingRadius(pos1, pos4, h2);
    assert(boundary == false && "0.5^2 = 0.25 must be outside (not <)");
    
    // Test 4: Just inside boundary
    glm::vec3 pos5(0.49f, 0, 0);
    bool just_inside = isWithinSmoothingRadius(pos1, pos5, h2);
    assert(just_inside == true && "0.49^2 = 0.2401 < 0.25 must be inside");
    
    // Test 5: Same position
    bool same = isWithinSmoothingRadius(pos1, pos1, h2);
    assert(same == true && "Same position (dist=0) must be within");
    
    // Test 6: 3D distance
    glm::vec3 pos6(0.1f, 0.1f, 0.1f);
    bool within_3d = isWithinSmoothingRadius(pos1, pos6, h2);
    float dist2_3d = 0.01f + 0.01f + 0.01f;
    assert(within_3d == (dist2_3d < h2) && "3D distance check");
    
    // Test 7: Different h2 values
    glm::vec3 pos7(0.3f, 0, 0);
    bool within_small = isWithinSmoothingRadius(pos1, pos7, 0.05f);
    bool within_large = isWithinSmoothingRadius(pos1, pos7, 0.15f);
    assert(within_small == false && "0.3^2 = 0.09 > 0.05");
    assert(within_large == true && "0.3^2 = 0.09 < 0.15");
    
    // Test 8: Negative coordinates
    glm::vec3 pos8(-0.1f, 0, 0);
    bool within_neg = isWithinSmoothingRadius(pos1, pos8, h2);
    assert(within_neg == true && "Negative coordinates: 0.1^2 = 0.01 < 0.25");
    
    std::cout << "All isWithinSmoothingRadius tests passed!" << std::endl;
    return 0;
}
