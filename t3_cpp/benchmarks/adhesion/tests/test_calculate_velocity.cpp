#include "../src/adhesion_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>

const double TOLERANCE = 1e-6;

bool approx_equal(double a, double b, double tol = TOLERANCE) {
    return std::abs(a - b) < tol;
}

int main() {
    // Test 1: Velocity with different weights - verify finite and time scaling
    RT rt1;
    Weighted_point p1(Point(0, 0, 0), 0.5);
    Weighted_point p2(Point(1, 0, 0), 1.0);
    Weighted_point p3(Point(0, 1, 0), 1.5);
    Weighted_point p4(Point(0, 0, 1), 2.0);
    
    rt1.insert(p1);
    rt1.insert(p2);
    rt1.insert(p3);
    rt1.insert(p4);
    
    auto cell1 = rt1.finite_cells_begin();
    
    // Test 1a: Velocity at time = 1.0
    Vector v1 = Adhesion::calculate_velocity(rt1, cell1, 1.0);
    assert(std::isfinite(v1.x()) && "Velocity x-component must be finite");
    assert(std::isfinite(v1.y()) && "Velocity y-component must be finite");
    assert(std::isfinite(v1.z()) && "Velocity z-component must be finite");
    
    double mag1 = std::sqrt(v1.x()*v1.x() + v1.y()*v1.y() + v1.z()*v1.z());
    assert(mag1 > 0 && "Velocity magnitude must be positive for non-degenerate cell");
    
    // Test 1b: Velocity at time = 2.0 - should scale inversely with time
    Vector v2 = Adhesion::calculate_velocity(rt1, cell1, 2.0);
    assert(std::isfinite(v2.x()) && std::isfinite(v2.y()) && std::isfinite(v2.z()));
    
    double mag2 = std::sqrt(v2.x()*v2.x() + v2.y()*v2.y() + v2.z()*v2.z());
    
    // Velocity should scale as 1/time, so v1 magnitude should be ~2x v2 magnitude
    double ratio = mag1 / mag2;
    assert(approx_equal(ratio, 2.0, 0.01) && "Velocity magnitude should scale inversely with time");
    
    // Test 1c: Component-wise scaling check
    // Each component should scale by the same factor (2.0)
    if (std::abs(v2.x()) > TOLERANCE) {
        double ratio_x = v1.x() / v2.x();
        assert(approx_equal(ratio_x, 2.0, 0.01) && "X-component should scale inversely with time");
    }
    if (std::abs(v2.y()) > TOLERANCE) {
        double ratio_y = v1.y() / v2.y();
        assert(approx_equal(ratio_y, 2.0, 0.01) && "Y-component should scale inversely with time");
    }
    if (std::abs(v2.z()) > TOLERANCE) {
        double ratio_z = v1.z() / v2.z();
        assert(approx_equal(ratio_z, 2.0, 0.01) && "Z-component should scale inversely with time");
    }
    
    // Test 2: Different time values - verify consistent scaling
    Vector v3 = Adhesion::calculate_velocity(rt1, cell1, 0.5);
    double mag3 = std::sqrt(v3.x()*v3.x() + v3.y()*v3.y() + v3.z()*v3.z());
    
    // v3 (time=0.5) should be 2x v1 (time=1.0)
    double ratio2 = mag3 / mag1;
    assert(approx_equal(ratio2, 2.0, 0.01) && "Velocity at time=0.5 should be 2x velocity at time=1.0");
    
    // Test 3: Symmetric configuration with equal weights
    RT rt2;
    Weighted_point q1(Point(0, 0, 0), 1.0);
    Weighted_point q2(Point(1, 0, 0), 1.0);
    Weighted_point q3(Point(0, 1, 0), 1.0);
    Weighted_point q4(Point(0, 0, 1), 1.0);
    
    rt2.insert(q1);
    rt2.insert(q2);
    rt2.insert(q3);
    rt2.insert(q4);
    
    auto cell2 = rt2.finite_cells_begin();
    Vector v4 = Adhesion::calculate_velocity(rt2, cell2, 1.0);
    
    assert(std::isfinite(v4.x()) && std::isfinite(v4.y()) && std::isfinite(v4.z()));
    double mag4 = std::sqrt(v4.x()*v4.x() + v4.y()*v4.y() + v4.z()*v4.z());
    assert(mag4 >= 0 && "Velocity magnitude must be non-negative");
    
    // Test 4: Verify time scaling with multiple time values
    Vector v5 = Adhesion::calculate_velocity(rt1, cell1, 4.0);
    double mag5 = std::sqrt(v5.x()*v5.x() + v5.y()*v5.y() + v5.z()*v5.z());
    
    // v1 (time=1.0) should be 4x v5 (time=4.0)
    double ratio3 = mag1 / mag5;
    assert(approx_equal(ratio3, 4.0, 0.01) && "Velocity should scale inversely with time factor of 4");
    
    // Test 5: Very small time value
    Vector v6 = Adhesion::calculate_velocity(rt1, cell1, 0.1);
    assert(std::isfinite(v6.x()) && std::isfinite(v6.y()) && std::isfinite(v6.z()));
    double mag6 = std::sqrt(v6.x()*v6.x() + v6.y()*v6.y() + v6.z()*v6.z());
    
    // v6 (time=0.1) should be 10x v1 (time=1.0)
    double ratio4 = mag6 / mag1;
    assert(approx_equal(ratio4, 10.0, 0.1) && "Velocity should scale inversely with time factor of 10");
    
    std::cout << "All calculate_velocity tests passed!" << std::endl;
    return 0;
}
