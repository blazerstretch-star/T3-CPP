#include "../src/adhesion_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

int main() {
    RT rt;
    
    // Test 1: Simple tetrahedron with known edge lengths
    // Edges: (0,0,0)-(1,0,0), (0,0,0)-(0,1,0), (0,0,0)-(0,0,1) have squared length 1.0
    // Edges: (1,0,0)-(0,1,0), (1,0,0)-(0,0,1), (0,1,0)-(0,0,1) have squared length 2.0
    Weighted_point p1(Point(0, 0, 0), 0);
    Weighted_point p2(Point(1, 0, 0), 0);
    Weighted_point p3(Point(0, 1, 0), 0);
    Weighted_point p4(Point(0, 0, 1), 0);
    
    rt.insert(p1);
    rt.insert(p2);
    rt.insert(p3);
    rt.insert(p4);
    
    auto cell = rt.finite_cells_begin();
    
    // Test 1a: threshold = 0.5 (all 6 edges should exceed: 3 edges with length^2=1.0, 3 with length^2=2.0)
    int count1 = Adhesion::edge_count(rt, cell, 0.5);
    assert(count1 == 6 && "All 6 edges should exceed threshold 0.5");
    
    // Test 1b: threshold = 1.5 (only 3 diagonal edges with length^2=2.0 should exceed)
    int count2 = Adhesion::edge_count(rt, cell, 1.5);
    assert(count2 == 3 && "Only 3 diagonal edges should exceed threshold 1.5");
    
    // Test 1c: threshold = 2.5 (no edges exceed)
    int count3 = Adhesion::edge_count(rt, cell, 2.5);
    assert(count3 == 0 && "No edges should exceed threshold 2.5");
    
    // Test 1d: threshold = 1.0 (only diagonal edges exceed, boundary case)
    int count4 = Adhesion::edge_count(rt, cell, 1.0);
    assert(count4 == 3 && "Only 3 diagonal edges should exceed threshold 1.0 (boundary)");
    
    // Test 1e: threshold = 2.0 (no edges exceed, boundary case)
    int count5 = Adhesion::edge_count(rt, cell, 2.0);
    assert(count5 == 0 && "No edges should exceed threshold 2.0 (boundary)");
    
    // Test 2: Different configuration with scaled edges
    RT rt2;
    Weighted_point q1(Point(0, 0, 0), 0);
    Weighted_point q2(Point(2, 0, 0), 0);
    Weighted_point q3(Point(0, 2, 0), 0);
    Weighted_point q4(Point(0, 0, 2), 0);
    
    rt2.insert(q1);
    rt2.insert(q2);
    rt2.insert(q3);
    rt2.insert(q4);
    
    auto cell2 = rt2.finite_cells_begin();
    
    // Edges: 3 with squared length 4.0, 3 with squared length 8.0
    int count6 = Adhesion::edge_count(rt2, cell2, 3.0);
    assert(count6 == 6 && "All 6 edges should exceed threshold 3.0");
    
    int count7 = Adhesion::edge_count(rt2, cell2, 5.0);
    assert(count7 == 3 && "Only 3 diagonal edges should exceed threshold 5.0");
    
    int count8 = Adhesion::edge_count(rt2, cell2, 9.0);
    assert(count8 == 0 && "No edges should exceed threshold 9.0");
    
    // Test 3: Edge case - threshold = 0 (all edges exceed)
    int count9 = Adhesion::edge_count(rt, cell, 0.0);
    assert(count9 == 6 && "All edges should exceed threshold 0.0");
    
    // Test 4: Very large threshold
    int count10 = Adhesion::edge_count(rt, cell, 1000.0);
    assert(count10 == 0 && "No edges should exceed very large threshold");
    
    std::cout << "All edge_count tests passed!" << std::endl;
    return 0;
}
