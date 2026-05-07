#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.0001f;
    
    // Test 1: Origin with h=0.5
    Particle p1;
    p1.position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::ivec3 cell1 = getCell(&p1, 0.5f);
    assert(cell1.x == 0 && cell1.y == 0 && cell1.z == 0 && "Origin must be cell (0,0,0)");
    
    // Test 2: Exact division
    Particle p2;
    p2.position = glm::vec3(1.0f, 2.0f, 3.0f);
    glm::ivec3 cell2 = getCell(&p2, 0.5f);
    assert(cell2.x == 2 && "1.0/0.5 = 2");
    assert(cell2.y == 4 && "2.0/0.5 = 4");
    assert(cell2.z == 6 && "3.0/0.5 = 6");
    
    // Test 3: Negative positions
    Particle p3;
    p3.position = glm::vec3(-1.0f, -2.0f, -3.0f);
    glm::ivec3 cell3 = getCell(&p3, 0.5f);
    assert(cell3.x == -2 && "-1.0/0.5 = -2");
    assert(cell3.y == -4 && "-2.0/0.5 = -4");
    assert(cell3.z == -6 && "-3.0/0.5 = -6");
    
    // Test 4: Within single cell
    Particle p4;
    p4.position = glm::vec3(0.25f, 0.25f, 0.25f);
    glm::ivec3 cell4 = getCell(&p4, 0.5f);
    assert(cell4.x == 0 && cell4.y == 0 && cell4.z == 0 && "0.25/0.5 = 0");
    
    // Test 5: Cell boundary
    Particle p5;
    p5.position = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::ivec3 cell5 = getCell(&p5, 0.5f);
    assert(cell5.x == 1 && cell5.y == 1 && cell5.z == 1 && "0.5/0.5 = 1");
    
    // Test 6: Different h value
    Particle p6;
    p6.position = glm::vec3(2.0f, 4.0f, 6.0f);
    glm::ivec3 cell6 = getCell(&p6, 1.0f);
    assert(cell6.x == 2 && cell6.y == 4 && cell6.z == 6 && "h=1.0 division");
    
    // Test 7: Large coordinates
    Particle p7;
    p7.position = glm::vec3(100.0f, 200.0f, 300.0f);
    glm::ivec3 cell7 = getCell(&p7, 0.5f);
    assert(cell7.x == 200 && cell7.y == 400 && cell7.z == 600 && "Large coordinates");
    
    // Test 8: Mixed positive/negative
    Particle p8;
    p8.position = glm::vec3(-1.5f, 2.5f, -3.5f);
    glm::ivec3 cell8 = getCell(&p8, 0.5f);
    assert(cell8.x == -3 && cell8.y == 5 && cell8.z == -7 && "Mixed signs");
    
    // Test 9: Small h value
    Particle p9;
    p9.position = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::ivec3 cell9 = getCell(&p9, 0.1f);
    assert(cell9.x == 10 && cell9.y == 10 && cell9.z == 10 && "Small h increases cell index");

    // Test 10: Negative non-integer position — cell index must be consistent with
    // the division pos/h truncated toward zero (C++ int cast behaviour).
    // Accept either floor or truncation: just verify the cell is adjacent to (0,0,0).
    Particle p10;
    p10.position = glm::vec3(-0.3f, -0.3f, -0.3f);
    glm::ivec3 cell10 = getCell(&p10, 1.0f);
    assert(cell10.x >= -1 && cell10.x <= 0 && "cell.x for -0.3/1.0 must be 0 or -1");
    assert(cell10.y >= -1 && cell10.y <= 0 && "cell.y for -0.3/1.0 must be 0 or -1");
    assert(cell10.z >= -1 && cell10.z <= 0 && "cell.z for -0.3/1.0 must be 0 or -1");

    // Test 11: Mixed-sign non-integer — verify each component is the correct integer
    // quotient (floor or truncation both acceptable for positive values).
    Particle p11;
    p11.position = glm::vec3(2.1f, -0.5f, 3.9f);
    glm::ivec3 cell11 = getCell(&p11, 1.0f);
    assert(cell11.x == 2 && "int(2.1/1.0) = 2");
    assert(cell11.y >= -1 && cell11.y <= 0 && "int(-0.5/1.0) is 0 (truncation) or -1 (floor)");
    assert(cell11.z == 3 && "int(3.9/1.0) = 3");

    // Test 12: Scaling with h — same position, different h
    Particle p12;
    p12.position = glm::vec3(3.0f, 3.0f, 3.0f);
    glm::ivec3 cell12a = getCell(&p12, 1.0f);
    glm::ivec3 cell12b = getCell(&p12, 2.0f);
    glm::ivec3 cell12c = getCell(&p12, 0.5f);
    assert(cell12a.x == 3 && cell12a.y == 3 && cell12a.z == 3 && "h=1.0: cell (3,3,3)");
    assert(cell12b.x == 1 && cell12b.y == 1 && cell12b.z == 1 && "h=2.0: cell (1,1,1)");
    assert(cell12c.x == 6 && cell12c.y == 6 && cell12c.z == 6 && "h=0.5: cell (6,6,6)");

    // Test 13: Cell adjacency invariant — particles < h apart must be in adjacent cells
    Particle pa, pb;
    pa.position = glm::vec3(0.9f, 0.9f, 0.9f);
    pb.position = glm::vec3(1.1f, 1.1f, 1.1f);
    glm::ivec3 ca = getCell(&pa, 1.0f);
    glm::ivec3 cb = getCell(&pb, 1.0f);
    assert(std::abs(cb.x - ca.x) <= 1 && "Adjacent cells: x diff <= 1");
    assert(std::abs(cb.y - ca.y) <= 1 && "Adjacent cells: y diff <= 1");
    assert(std::abs(cb.z - ca.z) <= 1 && "Adjacent cells: z diff <= 1");

    std::cout << "All getCell tests passed!" << std::endl;
    return 0;
}
