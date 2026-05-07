#include "../src/numerical_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    using namespace NumericalMethods;
    
    auto f1 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{x[0]};
    };
    std::vector<double> x0 = {1.0};
    std::vector<double> x1 = runge_kutta_step(x0, 0.0, 0.1, f1);
    assert(std::abs(x1[0] - std::exp(0.1)) < 1e-7);
    
    auto f2 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{-x[0]};
    };
    std::vector<double> x2_0 = {1.0};
    std::vector<double> x2_1 = runge_kutta_step(x2_0, 0.0, 0.1, f2);
    assert(std::abs(x2_1[0] - std::exp(-0.1)) < 1e-7);
    
    auto f3 = [](const std::vector<double>& x, double t) {
        return std::vector<double>{t};
    };
    std::vector<double> x3_0 = {0.0};
    std::vector<double> x3_1 = runge_kutta_step(x3_0, 0.0, 0.1, f3);
    assert(std::abs(x3_1[0] - 0.005) < 1e-7);
    
    auto f4 = [](const std::vector<double>& state, double t) {
        return std::vector<double>{state[1], -state[0]};
    };
    std::vector<double> x4_0 = {1.0, 0.0};
    std::vector<double> x4_1 = runge_kutta_step(x4_0, 0.0, 0.01, f4);
    assert(std::abs(x4_1[0] - std::cos(0.01)) < 1e-9);
    assert(std::abs(x4_1[1] - (-std::sin(0.01))) < 1e-9);
    
    std::cout << "test_runge_kutta_step PASSED" << std::endl;
    return 0;
}
