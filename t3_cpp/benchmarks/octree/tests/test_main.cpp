#include "../src/octree_functions.cpp"
#include <cassert>
#include <iostream>
#include <cmath>

void test_calculate_point_distance() {
    OrthoTree::Point3D p1 = {0.0, 0.0, 0.0};
    OrthoTree::Point3D p2 = {3.0, 4.0, 0.0};
    
    double distance = OrthoTree::calculate_point_distance(p1, p2);
    assert(std::abs(distance - 5.0) < 0.001);
    
    OrthoTree::Point3D p3 = {1.0, 1.0, 1.0};
    OrthoTree::Point3D p4 = {1.0, 1.0, 1.0};
    assert(OrthoTree::calculate_point_distance(p3, p4) == 0.0);
    
    std::cout << "test_calculate_point_distance: PASSED\n";
}

void test_is_point_in_box() {
    OrthoTree::BoundingBox3D box;
    box.Min = {0.0, 0.0, 0.0};
    box.Max = {10.0, 10.0, 10.0};
    
    OrthoTree::Point3D inside = {5.0, 5.0, 5.0};
    assert(OrthoTree::is_point_in_box(inside, box) == true);
    
    OrthoTree::Point3D outside = {15.0, 5.0, 5.0};
    assert(OrthoTree::is_point_in_box(outside, box) == false);
    
    OrthoTree::Point3D edge = {10.0, 10.0, 10.0};
    assert(OrthoTree::is_point_in_box(edge, box) == true);
    
    std::cout << "test_is_point_in_box: PASSED\n";
}

void test_calculate_box_volume() {
    OrthoTree::BoundingBox3D box;
    box.Min = {0.0, 0.0, 0.0};
    box.Max = {2.0, 3.0, 4.0};
    
    double volume = OrthoTree::calculate_box_volume(box);
    assert(std::abs(volume - 24.0) < 0.001);
    
    OrthoTree::BoundingBox3D unit_box;
    unit_box.Min = {0.0, 0.0, 0.0};
    unit_box.Max = {1.0, 1.0, 1.0};
    assert(std::abs(OrthoTree::calculate_box_volume(unit_box) - 1.0) < 0.001);
    
    std::cout << "test_calculate_box_volume: PASSED\n";
}

void test_find_nearest_point() {
    std::vector<OrthoTree::Point3D> points = {
        {0.0, 0.0, 0.0},
        {1.0, 1.0, 1.0},
        {5.0, 5.0, 5.0},
        {2.0, 2.0, 2.0}
    };
    
    OrthoTree::Point3D target = {1.5, 1.5, 1.5};
    OrthoTree::index_t nearest = OrthoTree::find_nearest_point(target, points);
    assert(nearest == 1);
    
    OrthoTree::Point3D target2 = {4.0, 4.0, 4.0};
    OrthoTree::index_t nearest2 = OrthoTree::find_nearest_point(target2, points);
    assert(nearest2 == 2);
    
    std::cout << "test_find_nearest_point: PASSED\n";
}

void test_boxes_overlap() {
    OrthoTree::BoundingBox3D box1;
    box1.Min = {0.0, 0.0, 0.0};
    box1.Max = {5.0, 5.0, 5.0};
    
    OrthoTree::BoundingBox3D box2;
    box2.Min = {3.0, 3.0, 3.0};
    box2.Max = {8.0, 8.0, 8.0};
    
    assert(OrthoTree::boxes_overlap(box1, box2) == true);
    
    OrthoTree::BoundingBox3D box3;
    box3.Min = {10.0, 10.0, 10.0};
    box3.Max = {15.0, 15.0, 15.0};
    
    assert(OrthoTree::boxes_overlap(box1, box3) == false);
    
    std::cout << "test_boxes_overlap: PASSED\n";
}

void test_calculate_box_center() {
    OrthoTree::BoundingBox3D box;
    box.Min = {0.0, 0.0, 0.0};
    box.Max = {10.0, 20.0, 30.0};
    
    OrthoTree::Point3D center = OrthoTree::calculate_box_center(box);
    assert(std::abs(center[0] - 5.0) < 0.001);
    assert(std::abs(center[1] - 10.0) < 0.001);
    assert(std::abs(center[2] - 15.0) < 0.001);
    
    std::cout << "test_calculate_box_center: PASSED\n";
}

int main() {
    std::cout << "Running Octree Function Tests...\n";
    
    test_calculate_point_distance();
    test_is_point_in_box();
    test_calculate_box_volume();
    test_find_nearest_point();
    test_boxes_overlap();
    test_calculate_box_center();
    
    std::cout << "All tests passed!\n";
    return 0;
}