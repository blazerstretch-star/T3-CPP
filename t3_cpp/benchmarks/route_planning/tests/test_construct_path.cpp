#include "../src/route_planning_functions.h"
#include <cassert>
#include <iostream>

using namespace RoutePlanning;

int main() {
    // Test 1: Basic linear path
    Node start(0.0f, 0.0f);
    Node middle(1.0f, 1.0f);
    Node end(2.0f, 2.0f);
    
    end.parent = &middle;
    middle.parent = &start;
    start.parent = nullptr;
    
    std::vector<Node> path = construct_path(&end);
    assert(path.size() == 3);
    assert(path[0].x == 0.0f && path[0].y == 0.0f);
    assert(path[2].x == 2.0f && path[2].y == 2.0f);
    
    // Test 2: Single node path
    Node single(5.0f, 5.0f);
    single.parent = nullptr;
    std::vector<Node> single_path = construct_path(&single);
    assert(single_path.size() == 1);
    assert(single_path[0].x == 5.0f && single_path[0].y == 5.0f);
    
    // Test 3: Long path
    Node n1(0.0f, 0.0f), n2(1.0f, 0.0f), n3(2.0f, 0.0f), n4(3.0f, 0.0f), n5(4.0f, 0.0f);
    n5.parent = &n4;
    n4.parent = &n3;
    n3.parent = &n2;
    n2.parent = &n1;
    n1.parent = nullptr;
    
    std::vector<Node> long_path = construct_path(&n5);
    assert(long_path.size() == 5);
    assert(long_path[0].x == 0.0f && long_path[4].x == 4.0f);
    
    // Test 4: Branching path verification
    Node root(0.0f, 0.0f);
    Node branch1(1.0f, 1.0f);
    Node branch2(2.0f, 1.0f);
    Node leaf(3.0f, 1.0f);
    
    leaf.parent = &branch2;
    branch2.parent = &branch1;
    branch1.parent = &root;
    root.parent = nullptr;
    
    std::vector<Node> branch_path = construct_path(&leaf);
    assert(branch_path.size() == 4);
    assert(branch_path[0].x == 0.0f && branch_path[3].x == 3.0f);
    
    // Test 5: Path ordering verification
    Node a(10.0f, 10.0f), b(20.0f, 20.0f), c(30.0f, 30.0f);
    c.parent = &b;
    b.parent = &a;
    a.parent = nullptr;
    
    std::vector<Node> ordered_path = construct_path(&c);
    for(size_t i = 1; i < ordered_path.size(); i++) {
        assert(ordered_path[i].x > ordered_path[i-1].x);
    }
    
    std::cout << "test_construct_path passed!" << std::endl;
    return 0;
}