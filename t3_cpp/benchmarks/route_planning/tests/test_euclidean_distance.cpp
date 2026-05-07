#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>
#include <limits>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic distance calculation
    Node node1(0.0f, 0.0f);
    Node node2(3.0f, 4.0f);
    assert(std::abs(euclidean_distance(node1, node2) - 5.0f) < 0.001f);
    
    // Test 2: Same point
    assert(euclidean_distance(node1, node1) == 0.0f);
    
    // Test 3: Negative coordinates
    Node node3(-3.0f, -4.0f);
    assert(std::abs(euclidean_distance(node1, node3) - 5.0f) < 0.001f);
    
    // Test 4: Symmetry test
    assert(std::abs(euclidean_distance(node1, node2) - euclidean_distance(node2, node1)) < 0.0001f);
    
    // Test 5: Large distances
    Node node4(1000.0f, 1000.0f);
    float expected = std::sqrt(2000000.0f);
    assert(std::abs(euclidean_distance(node1, node4) - expected) < 0.1f);
    
    // Test 6: Very small distances
    Node node5(0.001f, 0.001f);
    float small_dist = euclidean_distance(node1, node5);
    assert(small_dist > 0.0f && small_dist < 0.01f);
    
    // Test 7: Diagonal vs axis-aligned
    Node axis1(1.0f, 0.0f);
    Node axis2(0.0f, 1.0f);
    Node diag(1.0f, 1.0f);
    
    assert(euclidean_distance(node1, axis1) == 1.0f);
    assert(euclidean_distance(node1, axis2) == 1.0f);
    assert(std::abs(euclidean_distance(node1, diag) - std::sqrt(2.0f)) < 0.001f);
    
    // Test 8: Floating point precision
    Node precise1(0.123456f, 0.789012f);
    Node precise2(0.234567f, 0.890123f);
    float precise_dist = euclidean_distance(precise1, precise2);
    assert(precise_dist > 0.0f);
    
    // Test 9: Triangle inequality
    Node triangle1(0.0f, 0.0f);
    Node triangle2(1.0f, 0.0f);
    Node triangle3(0.5f, 0.5f);
    
    float d12 = euclidean_distance(triangle1, triangle2);
    float d13 = euclidean_distance(triangle1, triangle3);
    float d23 = euclidean_distance(triangle2, triangle3);
    
    assert(d12 <= d13 + d23 + 0.001f);
    assert(d13 <= d12 + d23 + 0.001f);
    assert(d23 <= d12 + d13 + 0.001f);
    
    std::cout << "test_euclidean_distance passed!" << std::endl;
    return 0;
}