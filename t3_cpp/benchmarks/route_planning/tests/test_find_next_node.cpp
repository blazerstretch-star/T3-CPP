#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic f-value sorting
    Node node1(0.0f, 0.0f);
    Node node2(1.0f, 1.0f);
    Node node3(2.0f, 2.0f);
    
    node1.g_value = 1.0f; node1.h_value = 3.0f; // f = 4
    node2.g_value = 2.0f; node2.h_value = 1.0f; // f = 3 (lowest)
    node3.g_value = 3.0f; node3.h_value = 2.0f; // f = 5
    
    std::vector<Node*> open_list = {&node1, &node2, &node3};
    Node* next = find_next_node(open_list);
    assert(next == &node2 && open_list.size() == 2);
    
    // Test 2: Tie-breaking (same f-value, prefer lower h-value)
    Node node4(3.0f, 3.0f);
    Node node5(4.0f, 4.0f);
    node4.g_value = 2.0f; node4.h_value = 2.0f; // f = 4
    node5.g_value = 1.0f; node5.h_value = 3.0f; // f = 4
    
    std::vector<Node*> tie_list = {&node4, &node5};
    Node* tie_result = find_next_node(tie_list);
    assert(tie_result->h_value <= 3.0f);
    
    // Test 3: Single node
    Node single(5.0f, 5.0f);
    single.g_value = 1.0f; single.h_value = 1.0f;
    std::vector<Node*> single_list = {&single};
    assert(find_next_node(single_list) == &single);
    assert(single_list.empty());
    
    // Test 4: Zero values
    Node zero1(0.0f, 0.0f), zero2(1.0f, 1.0f);
    zero1.g_value = 0.0f; zero1.h_value = 0.0f;
    zero2.g_value = 0.1f; zero2.h_value = 0.0f;
    std::vector<Node*> zero_list = {&zero2, &zero1};
    assert(find_next_node(zero_list) == &zero1);
    
    // Test 5: Large values
    Node large1(0.0f, 0.0f), large2(1.0f, 1.0f);
    large1.g_value = 1000.0f; large1.h_value = 1000.0f;
    large2.g_value = 999.0f; large2.h_value = 999.0f;
    std::vector<Node*> large_list = {&large1, &large2};
    assert(find_next_node(large_list) == &large2);
    
    std::cout << "test_find_next_node passed!" << std::endl;
    return 0;
}