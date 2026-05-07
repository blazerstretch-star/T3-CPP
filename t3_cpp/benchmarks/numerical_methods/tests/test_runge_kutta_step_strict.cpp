#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    // Test 1: dx/dt = x, solution is x(t) = x0 * e^t
    auto f1 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0]};
    };
    std::vector<double> x0 = {1.0};
    double h = 0.1;
    std::vector<double> x1 = runge_kutta_step(x0, 0.0, h, f1);
    assert(std::abs(x1[0] - std::exp(0.1)) < 1e-6);  // RK4 should be very accurate
    
    // Test 2: dx/dt = -x, solution is x(t) = x0 * e^(-t)
    auto f2 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{-x[0]};
    };
    std::vector<double> x2_0 = {1.0};
    std::vector<double> x2_1 = runge_kutta_step(x2_0, 0.0, h, f2);
    assert(std::abs(x2_1[0] - std::exp(-0.1)) < 1e-6);
    
    // Test 3: dx/dt = t, solution is x(t) = x0 + t^2/2
    auto f3 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{t};
    };
    std::vector<double> x3_0 = {0.0};
    std::vector<double> x3_1 = runge_kutta_step(x3_0, 0.0, h, f3);
    assert(std::abs(x3_1[0] - 0.1*0.1/2.0) < 1e-6);
    
    // Test 4: 2D system - harmonic oscillator
    // dx/dt = v, dv/dt = -x
    auto f4 = [](const std::vector<double>& state, double t) {
        return std::vector<double>{state[1], -state[0]};
    };
    std::vector<double> x4_0 = {1.0, 0.0};  // x=1, v=0
    std::vector<double> x4_1 = runge_kutta_step(x4_0, 0.0, 0.01, f4);
    // Analytical: x(t) = cos(t), v(t) = -sin(t)
    assert(std::abs(x4_1[0] - std::cos(0.01)) < 1e-8);
    assert(std::abs(x4_1[1] - (-std::sin(0.01))) < 1e-8);
    
    std::cout << "test_runge_kutta_step_strict PASSED" << std::endl;
    return 0;
}
