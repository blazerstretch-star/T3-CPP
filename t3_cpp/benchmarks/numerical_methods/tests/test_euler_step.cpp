#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // ========================================
    // PROPERTY 1: Simple ODE dx/dt = x, solution x(t) = x0 * e^t
    // ========================================
    auto f1 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0]};
    };
    
    std::vector<double> x0 = {1.0};
    double h = 0.1;
    std::vector<double> x1 = euler_step(x0, 0.0, h, f1);
    
    // Euler: x1 = x0 + h*f(x0) = 1.0 + 0.1*1.0 = 1.1
    assert(std::abs(x1[0] - 1.1) < 1e-10 && "Euler step should give 1.1");
    assert(x1.size() == 1 && "Output size should match input size");
    
    // ========================================
    // PROPERTY 2: ODE dx/dt = -x, solution x(t) = x0 * e^(-t)
    // ========================================
    auto f2 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{-x[0]};
    };
    
    std::vector<double> x2_0 = {1.0};
    std::vector<double> x2_1 = euler_step(x2_0, 0.0, h, f2);
    
    // Euler: x1 = x0 + h*(-x0) = 1.0 - 0.1*1.0 = 0.9
    assert(std::abs(x2_1[0] - 0.9) < 1e-10 && "Euler step should give 0.9");
    
    // ========================================
    // PROPERTY 3: ODE dx/dt = t, solution x(t) = x0 + t^2/2
    // ========================================
    auto f3 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{t};
    };
    
    std::vector<double> x3_0 = {0.0};
    std::vector<double> x3_1 = euler_step(x3_0, 0.0, h, f3);
    
    // Euler: x1 = x0 + h*t = 0.0 + 0.1*0.0 = 0.0
    assert(std::abs(x3_1[0]) < 1e-10 && "Euler step should give 0.0");
    
    // ========================================
    // PROPERTY 4: Constant derivative
    // ========================================
    auto f4 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{2.0};
    };
    
    std::vector<double> x4_0 = {5.0};
    std::vector<double> x4_1 = euler_step(x4_0, 0.0, 0.5, f4);
    
    // Euler: x1 = x0 + h*2.0 = 5.0 + 0.5*2.0 = 6.0
    assert(std::abs(x4_1[0] - 6.0) < 1e-10 && "Constant derivative should work");
    
    // ========================================
    // PROPERTY 5: 2D system - independent equations
    // ========================================
    auto f5 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0], -x[1]};
    };
    
    std::vector<double> x5_0 = {1.0, 2.0};
    std::vector<double> x5_1 = euler_step(x5_0, 0.0, 0.1, f5);
    
    // x1[0] = 1.0 + 0.1*1.0 = 1.1
    // x1[1] = 2.0 + 0.1*(-2.0) = 1.8
    assert(std::abs(x5_1[0] - 1.1) < 1e-10 && "First component should be 1.1");
    assert(std::abs(x5_1[1] - 1.8) < 1e-10 && "Second component should be 1.8");
    assert(x5_1.size() == 2 && "Output size should be 2");
    
    // ========================================
    // PROPERTY 6: 2D harmonic oscillator dx/dt = v, dv/dt = -x
    // ========================================
    auto f6 = [](const std::vector<double>& state, double t) {
        return std::vector<double>{state[1], -state[0]};
    };
    
    std::vector<double> x6_0 = {1.0, 0.0};  // x=1, v=0
    std::vector<double> x6_1 = euler_step(x6_0, 0.0, 0.01, f6);
    
    // x1 = 1.0 + 0.01*0.0 = 1.0
    // v1 = 0.0 + 0.01*(-1.0) = -0.01
    assert(std::abs(x6_1[0] - 1.0) < 1e-10 && "Position should be 1.0");
    assert(std::abs(x6_1[1] - (-0.01)) < 1e-10 && "Velocity should be -0.01");
    
    // ========================================
    // PROPERTY 7: Multiple steps consistency
    // ========================================
    auto f7 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{1.0};
    };
    
    std::vector<double> x7_0 = {0.0};
    std::vector<double> x7_1 = euler_step(x7_0, 0.0, 0.1, f7);
    std::vector<double> x7_2 = euler_step(x7_1, 0.1, 0.1, f7);
    
    // After 2 steps: 0.0 + 0.1 + 0.1 = 0.2
    assert(std::abs(x7_2[0] - 0.2) < 1e-10 && "Two steps should give 0.2");
    
    // ========================================
    // PROPERTY 8: Zero step size
    // ========================================
    auto f8 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0]};
    };
    
    std::vector<double> x8_0 = {5.0};
    std::vector<double> x8_1 = euler_step(x8_0, 0.0, 0.0, f8);
    
    // With h=0, x should not change
    assert(std::abs(x8_1[0] - 5.0) < 1e-10 && "Zero step should not change value");
    
    // ========================================
    // PROPERTY 9: 3D system
    // ========================================
    auto f9 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0], x[1], x[2]};
    };
    
    std::vector<double> x9_0 = {1.0, 2.0, 3.0};
    std::vector<double> x9_1 = euler_step(x9_0, 0.0, 0.1, f9);
    
    assert(std::abs(x9_1[0] - 1.1) < 1e-10 && "First component correct");
    assert(std::abs(x9_1[1] - 2.2) < 1e-10 && "Second component correct");
    assert(std::abs(x9_1[2] - 3.3) < 1e-10 && "Third component correct");
    assert(x9_1.size() == 3 && "Output size should be 3");
    
    // ========================================
    // PROPERTY 10: Negative step size (backward integration)
    // ========================================
    auto f10 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{1.0};
    };
    
    std::vector<double> x10_0 = {5.0};
    std::vector<double> x10_1 = euler_step(x10_0, 1.0, -0.1, f10);
    
    // x1 = 5.0 + (-0.1)*1.0 = 4.9
    assert(std::abs(x10_1[0] - 4.9) < 1e-10 && "Negative step should work");
    
    std::cout << "test_euler_step PASSED" << std::endl;
    return 0;
}
