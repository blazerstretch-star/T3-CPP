#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

using namespace nanort;

// ============================================================================
// PROPERTY-BASED TESTS: Verify mathematical invariants
// ============================================================================

// PROPERTY 1: Commutativity - dot(a, b) = dot(b, a)
TEST(VdotTest, Property_Commutativity) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    float dot_ab = vdot(a, b);
    float dot_ba = vdot(b, a);
    
    EXPECT_FLOAT_EQ(dot_ab, dot_ba) << "Dot product should be commutative: dot(a,b) = dot(b,a)";
    
    // Test with different vectors
    real3<float> c(-2.0f, 7.0f, -3.0f);
    real3<float> d(5.0f, -1.0f, 8.0f);
    
    EXPECT_FLOAT_EQ(vdot(c, d), vdot(d, c)) << "Commutativity should hold for all vectors";
}

// PROPERTY 2: Distributivity - dot(a, b+c) = dot(a,b) + dot(a,c)
TEST(VdotTest, Property_Distributivity) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    real3<float> c(7.0f, 8.0f, 9.0f);
    
    // Compute b + c manually
    real3<float> b_plus_c(b[0] + c[0], b[1] + c[1], b[2] + c[2]);
    
    float dot_a_bc = vdot(a, b_plus_c);
    float dot_ab_plus_ac = vdot(a, b) + vdot(a, c);
    
    EXPECT_NEAR(dot_a_bc, dot_ab_plus_ac, 1e-5f) 
        << "Dot product should be distributive: dot(a, b+c) = dot(a,b) + dot(a,c)";
}

// PROPERTY 3: Scalar multiplication - dot(k*a, b) = k * dot(a,b)
TEST(VdotTest, Property_ScalarMultiplication) {
    real3<float> a(2.0f, 3.0f, 4.0f);
    real3<float> b(5.0f, 6.0f, 7.0f);
    float k = 3.5f;
    
    real3<float> ka = k * a;
    float dot_ka_b = vdot(ka, b);
    float k_dot_ab = k * vdot(a, b);
    
    EXPECT_NEAR(dot_ka_b, k_dot_ab, 1e-5f) 
        << "Scalar multiplication: dot(k*a, b) = k * dot(a,b)";
    
    // Test with negative scalar
    float k2 = -2.0f;
    real3<float> k2a = k2 * a;
    EXPECT_NEAR(vdot(k2a, b), k2 * vdot(a, b), 1e-5f)
        << "Should work with negative scalars";
}

// PROPERTY 4: Self-dot equals length squared - dot(v, v) = length(v)²
TEST(VdotTest, Property_SelfDotEqualsLengthSquared) {
    real3<float> v(3.0f, 4.0f, 0.0f);
    
    float dot_vv = vdot(v, v);
    float length_v = vlength(v);
    float length_squared = length_v * length_v;
    
    EXPECT_NEAR(dot_vv, length_squared, 1e-5f) 
        << "Self-dot should equal length squared: dot(v,v) = length(v)²";
    
    // Test with different vector
    real3<float> w(1.0f, 2.0f, 3.0f);
    float dot_ww = vdot(w, w);
    float length_w_sq = vlength(w) * vlength(w);
    
    EXPECT_NEAR(dot_ww, length_w_sq, 1e-5f)
        << "Property should hold for all vectors";
}

// PROPERTY 5: Cauchy-Schwarz inequality - |dot(a,b)| ≤ length(a) * length(b)
TEST(VdotTest, Property_CauchySchwarzInequality) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    float dot_ab = std::abs(vdot(a, b));
    float product_lengths = vlength(a) * vlength(b);
    
    EXPECT_LE(dot_ab, product_lengths + 1e-5f) 
        << "Cauchy-Schwarz: |dot(a,b)| ≤ length(a) * length(b)";
    
    // Test with various vectors
    real3<float> c(-5.0f, 3.0f, -2.0f);
    real3<float> d(7.0f, -1.0f, 4.0f);
    
    EXPECT_LE(std::abs(vdot(c, d)), vlength(c) * vlength(d) + 1e-5f)
        << "Inequality should hold for all vectors";
}

// PROPERTY 6: Orthogonal vectors - dot(a,b) = 0 implies a ⊥ b
TEST(VdotTest, Property_OrthogonalVectors) {
    // Standard basis vectors are orthogonal
    real3<float> x_axis(1.0f, 0.0f, 0.0f);
    real3<float> y_axis(0.0f, 1.0f, 0.0f);
    real3<float> z_axis(0.0f, 0.0f, 1.0f);
    
    EXPECT_NEAR(vdot(x_axis, y_axis), 0.0f, 1e-6f) << "X and Y axes should be orthogonal";
    EXPECT_NEAR(vdot(x_axis, z_axis), 0.0f, 1e-6f) << "X and Z axes should be orthogonal";
    EXPECT_NEAR(vdot(y_axis, z_axis), 0.0f, 1e-6f) << "Y and Z axes should be orthogonal";
    
    // Custom orthogonal vectors
    real3<float> v1(1.0f, 1.0f, 0.0f);
    real3<float> v2(-1.0f, 1.0f, 0.0f);
    
    EXPECT_NEAR(vdot(v1, v2), 0.0f, 1e-6f) << "Orthogonal vectors should have zero dot product";
}

// PROPERTY 7: Parallel vectors - dot(a, a/|a|) = |a|
TEST(VdotTest, Property_ParallelVectors) {
    real3<float> v(3.0f, 4.0f, 5.0f);
    real3<float> v_normalized = vnormalize(v);
    
    float dot_result = vdot(v, v_normalized);
    float length_v = vlength(v);
    
    EXPECT_NEAR(dot_result, length_v, 1e-5f) 
        << "Dot product of vector with its normalized version equals its length";
}

// PROPERTY 8: Anti-parallel vectors - dot(a, -a) = -dot(a, a)
TEST(VdotTest, Property_AntiParallelVectors) {
    real3<float> v(2.0f, 3.0f, 4.0f);
    real3<float> neg_v = vneg(v);
    
    float dot_v_negv = vdot(v, neg_v);
    float dot_v_v = vdot(v, v);
    
    EXPECT_NEAR(dot_v_negv, -dot_v_v, 1e-5f) 
        << "Dot product with negated vector should be negative of self-dot";
}

// ============================================================================
// EDGE CASE TESTS: Boundary conditions
// ============================================================================

// EDGE CASE 1: Zero vector
TEST(VdotTest, EdgeCase_ZeroVector) {
    real3<float> zero(0.0f, 0.0f, 0.0f);
    real3<float> v(1.0f, 2.0f, 3.0f);
    
    EXPECT_FLOAT_EQ(vdot(zero, v), 0.0f) << "Dot product with zero vector should be zero";
    EXPECT_FLOAT_EQ(vdot(v, zero), 0.0f) << "Dot product with zero vector should be zero";
    EXPECT_FLOAT_EQ(vdot(zero, zero), 0.0f) << "Zero dot zero should be zero";
}

// EDGE CASE 2: Unit vectors
TEST(VdotTest, EdgeCase_UnitVectors) {
    real3<float> unit_x(1.0f, 0.0f, 0.0f);
    real3<float> unit_y(0.0f, 1.0f, 0.0f);
    real3<float> unit_z(0.0f, 0.0f, 1.0f);
    
    // Self-dot of unit vector should be 1
    EXPECT_FLOAT_EQ(vdot(unit_x, unit_x), 1.0f) << "Unit vector self-dot should be 1";
    EXPECT_FLOAT_EQ(vdot(unit_y, unit_y), 1.0f) << "Unit vector self-dot should be 1";
    EXPECT_FLOAT_EQ(vdot(unit_z, unit_z), 1.0f) << "Unit vector self-dot should be 1";
}

// EDGE CASE 3: Parallel vectors (same direction)
TEST(VdotTest, EdgeCase_ParallelVectorsSameDirection) {
    real3<float> v1(2.0f, 4.0f, 6.0f);
    real3<float> v2(1.0f, 2.0f, 3.0f);  // v1 = 2 * v2
    
    float dot_result = vdot(v1, v2);
    float expected = vlength(v1) * vlength(v2);  // Maximum possible value
    
    EXPECT_NEAR(dot_result, expected, 1e-5f) 
        << "Parallel vectors (same direction) should have dot = |a|*|b|";
}

// EDGE CASE 4: Parallel vectors (opposite direction)
TEST(VdotTest, EdgeCase_ParallelVectorsOppositeDirection) {
    real3<float> v1(2.0f, 4.0f, 6.0f);
    real3<float> v2(-1.0f, -2.0f, -3.0f);  // v1 = -2 * v2
    
    float dot_result = vdot(v1, v2);
    float expected = -vlength(v1) * vlength(v2);  // Minimum possible value
    
    EXPECT_NEAR(dot_result, expected, 1e-5f) 
        << "Parallel vectors (opposite direction) should have dot = -|a|*|b|";
}

// EDGE CASE 5: Large magnitude vectors
TEST(VdotTest, EdgeCase_LargeMagnitude) {
    real3<float> large1(1e10f, 2e10f, 3e10f);
    real3<float> large2(4e10f, 5e10f, 6e10f);
    
    float dot_result = vdot(large1, large2);
    
    // Should not overflow to infinity
    EXPECT_FALSE(std::isinf(dot_result)) << "Large vector dot product should not overflow";
    EXPECT_FALSE(std::isnan(dot_result)) << "Large vector dot product should not be NaN";
    
    // Should still satisfy Cauchy-Schwarz
    EXPECT_LE(std::abs(dot_result), vlength(large1) * vlength(large2) + 1e5f);
}

// EDGE CASE 6: Very small magnitude vectors
TEST(VdotTest, EdgeCase_SmallMagnitude) {
    real3<float> small1(1e-10f, 2e-10f, 3e-10f);
    real3<float> small2(4e-10f, 5e-10f, 6e-10f);
    
    float dot_result = vdot(small1, small2);
    
    // Should not underflow to zero (unless truly zero)
    EXPECT_FALSE(std::isnan(dot_result)) << "Small vector dot product should not be NaN";
    
    // Should be very small but computable
    EXPECT_GE(dot_result, 0.0f) << "Dot product of positive vectors should be positive";
}

// EDGE CASE 7: Mixed positive and negative components
TEST(VdotTest, EdgeCase_MixedComponents) {
    real3<float> mixed1(1.0f, -2.0f, 3.0f);
    real3<float> mixed2(-4.0f, 5.0f, -6.0f);
    
    // Hand-calculated: 1*(-4) + (-2)*5 + 3*(-6) = -4 - 10 - 18 = -32
    float dot_result = vdot(mixed1, mixed2);
    
    EXPECT_FLOAT_EQ(dot_result, -32.0f) << "Mixed component dot product";
    
    // Should still satisfy commutativity
    EXPECT_FLOAT_EQ(vdot(mixed1, mixed2), vdot(mixed2, mixed1));
}

// ============================================================================
// KNOWN VALUE TESTS: Hand-calculated results
// ============================================================================

TEST(VdotTest, KnownValue_Orthogonal) {
    real3<float> a(1.0f, 0.0f, 0.0f);
    real3<float> b(0.0f, 1.0f, 0.0f);
    
    EXPECT_FLOAT_EQ(vdot(a, b), 0.0f) << "Orthogonal vectors should have zero dot product";
}

TEST(VdotTest, KnownValue_General) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_FLOAT_EQ(vdot(a, b), 32.0f);
}

TEST(VdotTest, KnownValue_Negative) {
    real3<float> a(-1.0f, -2.0f, -3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    // (-1)*4 + (-2)*5 + (-3)*6 = -4 - 10 - 18 = -32
    EXPECT_FLOAT_EQ(vdot(a, b), -32.0f);
}

TEST(VdotTest, KnownValue_SelfDot) {
    real3<float> v(3.0f, 4.0f, 0.0f);
    
    // 3*3 + 4*4 + 0*0 = 9 + 16 = 25
    EXPECT_FLOAT_EQ(vdot(v, v), 25.0f);
}
