#include "../src/cppqed_functions.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    const double tol = 1e-10;
    
    // Test 1: Simple 3x3 sparse matrix
    std::vector<dcomp> psi = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0)};
    std::vector<dcomp> dpsidt = {dcomp(0.0, 0.0), dcomp(0.0, 0.0), dcomp(0.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements;
    elements.push_back({0, 1, dcomp(1.0, 0.0)});  // dpsidt[0] += 1.0 * psi[1]
    elements.push_back({1, 2, dcomp(2.0, 0.0)});  // dpsidt[1] += 2.0 * psi[2]
    
    apply_sparse_matrix(psi, dpsidt, elements);
    
    assert(std::abs(dpsidt[0] - dcomp(2.0, 0.0)) < tol);
    assert(std::abs(dpsidt[1] - dcomp(6.0, 0.0)) < tol);
    assert(std::abs(dpsidt[2] - dcomp(0.0, 0.0)) < tol);
    
    // Test 2: Complex values
    std::vector<dcomp> psi2 = {dcomp(1.0, 1.0), dcomp(0.0, 1.0)};
    std::vector<dcomp> dpsidt2 = {dcomp(0.0, 0.0), dcomp(0.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements2;
    elements2.push_back({0, 1, dcomp(0.0, 1.0)});  // i * psi[1] = i * i = -1
    
    apply_sparse_matrix(psi2, dpsidt2, elements2);
    
    assert(std::abs(dpsidt2[0] - dcomp(-1.0, 0.0)) < tol);
    
    // Test 3: Accumulation - non-zero initial dpsidt
    std::vector<dcomp> psi3 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0)};
    std::vector<dcomp> dpsidt3 = {dcomp(5.0, 0.0), dcomp(3.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements3;
    elements3.push_back({0, 1, dcomp(2.0, 0.0)});  // dpsidt[0] += 2.0 * 2.0 = 4.0
    
    apply_sparse_matrix(psi3, dpsidt3, elements3);
    
    assert(std::abs(dpsidt3[0] - dcomp(9.0, 0.0)) < tol);  // 5.0 + 4.0
    assert(std::abs(dpsidt3[1] - dcomp(3.0, 0.0)) < tol);  // unchanged
    
    // Test 4: Multiple elements affecting same row
    std::vector<dcomp> psi4 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0), dcomp(3.0, 0.0)};
    std::vector<dcomp> dpsidt4 = {dcomp(0.0, 0.0), dcomp(0.0, 0.0), dcomp(0.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements4;
    elements4.push_back({0, 0, dcomp(1.0, 0.0)});  // dpsidt[0] += 1.0 * 1.0
    elements4.push_back({0, 1, dcomp(2.0, 0.0)});  // dpsidt[0] += 2.0 * 2.0
    elements4.push_back({0, 2, dcomp(3.0, 0.0)});  // dpsidt[0] += 3.0 * 3.0
    
    apply_sparse_matrix(psi4, dpsidt4, elements4);
    
    assert(std::abs(dpsidt4[0] - dcomp(14.0, 0.0)) < tol);  // 1 + 4 + 9
    
    // Test 5: Empty sparse matrix
    std::vector<dcomp> psi5 = {dcomp(1.0, 0.0), dcomp(2.0, 0.0)};
    std::vector<dcomp> dpsidt5 = {dcomp(3.0, 0.0), dcomp(4.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements5;  // Empty
    
    apply_sparse_matrix(psi5, dpsidt5, elements5);
    
    assert(std::abs(dpsidt5[0] - dcomp(3.0, 0.0)) < tol);  // unchanged
    assert(std::abs(dpsidt5[1] - dcomp(4.0, 0.0)) < tol);  // unchanged
    
    // Test 6: Single element
    std::vector<dcomp> psi6 = {dcomp(5.0, 2.0)};
    std::vector<dcomp> dpsidt6 = {dcomp(0.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements6;
    elements6.push_back({0, 0, dcomp(1.0, 1.0)});
    
    apply_sparse_matrix(psi6, dpsidt6, elements6);
    
    // (1+i)*(5+2i) = 5+2i+5i+2i^2 = 5+7i-2 = 3+7i
    assert(std::abs(dpsidt6[0] - dcomp(3.0, 7.0)) < tol);
    
    // Test 7: Complex matrix element and complex state
    std::vector<dcomp> psi7 = {dcomp(1.0, 2.0), dcomp(3.0, 4.0)};
    std::vector<dcomp> dpsidt7 = {dcomp(0.0, 0.0), dcomp(0.0, 0.0)};
    
    std::list<std::tuple<size_t, size_t, dcomp>> elements7;
    elements7.push_back({1, 0, dcomp(2.0, -1.0)});
    
    apply_sparse_matrix(psi7, dpsidt7, elements7);
    
    // (2-i)*(1+2i) = 2+4i-i-2i^2 = 2+3i+2 = 4+3i
    assert(std::abs(dpsidt7[1] - dcomp(4.0, 3.0)) < tol);
    
    std::cout << "All tests passed for apply_sparse_matrix!" << std::endl;
    return 0;
}
