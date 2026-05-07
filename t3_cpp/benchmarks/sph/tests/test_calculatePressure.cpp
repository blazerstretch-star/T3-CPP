#include "../src/sph_functions.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
    const float EPSILON = 0.00001f;
    
    // Test 1: Exact calculation
    float density = 1000.0f;
    float restDensity = 998.0f;
    float gasConstant = 2000.0f;
    float pressure = calculatePressure(density, restDensity, gasConstant);
    float expected = gasConstant * (density - restDensity);
    assert(std::abs(pressure - expected) < EPSILON && "Pressure calculation mismatch");
    assert(std::abs(pressure - 4000.0f) < EPSILON && "Expected 2000*(1000-998)=4000");
    
    // Test 2: Higher density gives higher pressure
    float pressure2 = calculatePressure(1100.0f, restDensity, gasConstant);
    assert(pressure2 > pressure && "Higher density must give higher pressure");
    float expected2 = 2000.0f * (1100.0f - 998.0f);
    assert(std::abs(pressure2 - expected2) < EPSILON && "High density pressure mismatch");
    
    // Test 3: At rest density, pressure is zero
    float pressure3 = calculatePressure(998.0f, 998.0f, gasConstant);
    assert(std::abs(pressure3) < EPSILON && "Rest density must give zero pressure");
    
    // Test 4: Below rest density gives negative pressure
    float pressure4 = calculatePressure(900.0f, 998.0f, gasConstant);
    assert(pressure4 < 0 && "Below rest density must give negative pressure");
    float expected4 = 2000.0f * (900.0f - 998.0f);
    assert(std::abs(pressure4 - expected4) < EPSILON && "Negative pressure mismatch");
    
    // Test 5: Linear gas constant scaling
    float pressure5a = calculatePressure(1000.0f, 998.0f, 1000.0f);
    float pressure5b = calculatePressure(1000.0f, 998.0f, 2000.0f);
    assert(std::abs(pressure5b / pressure5a - 2.0f) < 0.01f && "Gas constant scaling violated");
    
    // Test 6: Linear density difference scaling
    float pressure6a = calculatePressure(1000.0f, 998.0f, 2000.0f);
    float pressure6b = calculatePressure(1004.0f, 998.0f, 2000.0f);
    assert(std::abs(pressure6b / pressure6a - 3.0f) < 0.01f && "Density difference scaling violated");
    
    // Test 7: Different rest density
    float pressure7 = calculatePressure(1000.0f, 1000.0f, 2000.0f);
    assert(std::abs(pressure7) < EPSILON && "Matching rest density must give zero");
    
    std::cout << "All calculatePressure tests passed!" << std::endl;
    return 0;
}
