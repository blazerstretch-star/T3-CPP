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
    assert(std::abs(calculate_h_value(&node1, &node2) - 5.0f) < 0.001f);
    
    // Test 2: Same point
    assert(calculate_h_value(&node1, &node1) == 0.0f);
    
    // Test 3: Negative coordinates
    Node node3(-3.0f, -4.0f);
    assert(std::abs(calculate_h_value(&node1, &node3) - 5.0f) < 0.001f);
    
    // Test 4: Large distances
    Node node4(1000.0f, 1000.0f);
    float expected = std::sqrt(2000000.0f);
    assert(std::abs(calculate_h_value(&node1, &node4) - expected) < 0.1f);
    
    // Test 5: Very small distances
    Node node5(0.001f, 0.001f);
    float small_dist = calculate_h_value(&node1, &node5);
    assert(small_dist > 0.0f && small_dist < 0.01f);
    
    // Test 6: Floating point precision
    Node node6(0.1f, 0.1f);
    Node node7(0.2f, 0.2f);
    float precise_dist = calculate_h_value(&node6, &node7);
    assert(std::abs(precise_dist - std::sqrt(0.02f)) < 0.0001f);
    
    std::cout << "test_calculate_h_value passed!" << std::endl;
    return 0;
}