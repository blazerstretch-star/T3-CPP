#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

using namespace nanort;

// ============================================================================
// PROPERTY-BASED TESTS: Verify mathematical invariants
// ============================================================================

// PROPERTY 1: Anti-commutativity - cross(a, b) = -cross(b, a)
TEST(VcrossTest, Property_AntiCommutativity) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    real3<float> cross_ab = vcross(a, b);
    real3<float> cross_ba = vcross(b, a);
    
    EXPECT_NEAR(cross_ab[0], -cross_ba[0], 1e-5f) 
        << "Cross product should be anti-commutative: cross(a,b) = -cross(b,a)";
    EXPECT_NEAR(cross_ab[1], -cross_ba[1], 1e-5f) 
        << "Cross product should be anti-commutative: cross(a,b) = -cross(b,a)";
    EXPECT_NEAR(cross_ab[2], -cross_ba[2], 1e-5f) 
        << "Cross product should be anti-commutative: cross(a,b) = -cross(b,a)";
    
    // Test with different vectors
    real3<float> c(-2.0f, 7.0f, -3.0f);
    real3<float> d(5.0f, -1.0f, 8.0f);
    
    real3<float> cross_cd = vcross(c, d);
    real3<float> cross_dc = vcross(d, c);
    
    EXPECT_NEAR(cross_cd[0], -cross_dc[0], 1e-5f);
    EXPECT_NEAR(cross_cd[1], -cross_dc[1], 1e-5f);
    EXPECT_NEAR(cross_cd[2], -cross_dc[2], 1e-5f);
}

// PROPERTY 2: Perpendicularity - cross(a,b) ⊥ a and cross(a,b) ⊥ b
TEST(VcrossTest, Property_Perpendicularity) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    real3<float> cross_result = vcross(a, b);
    
    // Cross product should be perpendicular to both input vectors
    float dot_cross_a = vdot(cross_result, a);
    float dot_cross_b = vdot(cross_result, b);
    
    EXPECT_NEAR(dot_cross_a, 0.0f, 1e-4f) 
        << "Cross product should be perpendicular to first vector: dot(cross(a,b), a) = 0";
    EXPECT_NEAR(dot_cross_b, 0.0f, 1e-4f) 
        << "Cross product should be perpendicular to second vector: dot(cross(a,b), b) = 0";
    
    // Test with different vectors
    real3<float> c(3.0f, -1.0f, 2.0f);
    real3<float> d(-2.0f, 4.0f, 1.0f);
    
    real3<float> cross_cd = vcross(c, d);
    
    EXPECT_NEAR(vdot(cross_cd, c), 0.0f, 1e-4f) << "Should be perpendicular to c";
    EXPECT_NEAR(vdot(cross_cd, d), 0.0f, 1e-4f) << "Should be perpendicular to d";
}

// PROPERTY 3: Distributivity - cross(a, b+c) = cross(a,b) + cross(a,c)
TEST(VcrossTest, Property_Distributivity) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    real3<float> c(7.0f, 8.0f, 9.0f);
    
    // Compute b + c manually
    real3<float> b_plus_c(b[0] + c[0], b[1] + c[1], b[2] + c[2]);
    
    real3<float> cross_a_bc = vcross(a, b_plus_c);
    
    real3<float> cross_ab = vcross(a, b);
    real3<float> cross_ac = vcross(a, c);
    real3<float> cross_ab_plus_ac(cross_ab[0] + cross_ac[0], 
                                   cross_ab[1] + cross_ac[1], 
                                   cross_ab[2] + cross_ac[2]);
    
    EXPECT_NEAR(cross_a_bc[0], cross_ab_plus_ac[0], 1e-4f) 
        << "Cross product should be distributive: cross(a, b+c) = cross(a,b) + cross(a,c)";
    EXPECT_NEAR(cross_a_bc[1], cross_ab_plus_ac[1], 1e-4f);
    EXPECT_NEAR(cross_a_bc[2], cross_ab_plus_ac[2], 1e-4f);
}

// PROPERTY 4: Scalar multiplication - cross(k*a, b) = k * cross(a,b)
TEST(VcrossTest, Property_ScalarMultiplication) {
    real3<float> a(2.0f, 3.0f, 4.0f);
    real3<float> b(5.0f, 6.0f, 7.0f);
    float k = 3.5f;
    
    real3<float> ka = k * a;
    real3<float> cross_ka_b = vcross(ka, b);
    
    real3<float> cross_ab = vcross(a, b);
    real3<float> k_cross_ab = k * cross_ab;
    
    EXPECT_NEAR(cross_ka_b[0], k_cross_ab[0], 1e-4f) 
        << "Scalar multiplication: cross(k*a, b) = k * cross(a,b)";
    EXPECT_NEAR(cross_ka_b[1], k_cross_ab[1], 1e-4f);
    EXPECT_NEAR(cross_ka_b[2], k_cross_ab[2], 1e-4f);
    
    // Test with negative scalar
    float k2 = -2.0f;
    real3<float> k2a = k2 * a;
    real3<float> cross_k2a_b = vcross(k2a, b);
    real3<float> k2_cross_ab = k2 * cross_ab;
    
    EXPECT_NEAR(cross_k2a_b[0], k2_cross_ab[0], 1e-4f);
    EXPECT_NEAR(cross_k2a_b[1], k2_cross_ab[1], 1e-4f);
    EXPECT_NEAR(cross_k2a_b[2], k2_cross_ab[2], 1e-4f);
}

// PROPERTY 5: Right-hand rule - cross(x, y) = z for standard basis
TEST(VcrossTest, Property_RightHandRule) {
    real3<float> x_axis(1.0f, 0.0f, 0.0f);
    real3<float> y_axis(0.0f, 1.0f, 0.0f);
    real3<float> z_axis(0.0f, 0.0f, 1.0f);
    
    real3<float> cross_xy = vcross(x_axis, y_axis);
    EXPECT_NEAR(cross_xy[0], z_axis[0], 1e-6f) << "cross(x, y) should equal z";
    EXPECT_NEAR(cross_xy[1], z_axis[1], 1e-6f);
    EXPECT_NEAR(cross_xy[2], z_axis[2], 1e-6f);
    
    real3<float> cross_yz = vcross(y_axis, z_axis);
    EXPECT_NEAR(cross_yz[0], x_axis[0], 1e-6f) << "cross(y, z) should equal x";
    EXPECT_NEAR(cross_yz[1], x_axis[1], 1e-6f);
    EXPECT_NEAR(cross_yz[2], x_axis[2], 1e-6f);
    
    real3<float> cross_zx = vcross(z_axis, x_axis);
    EXPECT_NEAR(cross_zx[0], y_axis[0], 1e-6f) << "cross(z, x) should equal y";
    EXPECT_NEAR(cross_zx[1], y_axis[1], 1e-6f);
    EXPECT_NEAR(cross_zx[2], y_axis[2], 1e-6f);
}

// PROPERTY 6: Parallel vectors - cross(a, k*a) = 0
TEST(VcrossTest, Property_ParallelVectors) {
    real3<float> v(2.0f, 3.0f, 4.0f);
    real3<float> parallel = 5.0f * v;
    
    real3<float> cross_result = vcross(v, parallel);
    
    EXPECT_NEAR(vlength(cross_result), 0.0f, 1e-5f) 
        << "Cross product of parallel vectors should be zero";
    
    // Test with negative parallel
    real3<float> anti_parallel = -3.0f * v;
    real3<float> cross_result2 = vcross(v, anti_parallel);
    
    EXPECT_NEAR(vlength(cross_result2), 0.0f, 1e-5f) 
        << "Cross product of anti-parallel vectors should be zero";
}

// PROPERTY 7: Magnitude relationship - |cross(a,b)| = |a| * |b| * sin(θ)
TEST(VcrossTest, Property_MagnitudeRelationship) {
    // For orthogonal vectors, sin(90°) = 1, so |cross(a,b)| = |a| * |b|
    real3<float> a(3.0f, 0.0f, 0.0f);
    real3<float> b(0.0f, 4.0f, 0.0f);
    
    real3<float> cross_result = vcross(a, b);
    float cross_magnitude = vlength(cross_result);
    float expected_magnitude = vlength(a) * vlength(b);  // sin(90°) = 1
    
    EXPECT_NEAR(cross_magnitude, expected_magnitude, 1e-5f) 
        << "For orthogonal vectors: |cross(a,b)| = |a| * |b|";
}

// PROPERTY 8: Self-cross is zero - cross(a, a) = 0
TEST(VcrossTest, Property_SelfCrossIsZero) {
    real3<float> v(5.0f, 7.0f, 9.0f);
    
    real3<float> cross_result = vcross(v, v);
    
    EXPECT_NEAR(cross_result[0], 0.0f, 1e-6f) << "Self-cross should be zero";
    EXPECT_NEAR(cross_result[1], 0.0f, 1e-6f) << "Self-cross should be zero";
    EXPECT_NEAR(cross_result[2], 0.0f, 1e-6f) << "Self-cross should be zero";
}

// ============================================================================
// EDGE CASE TESTS: Boundary conditions
// ============================================================================

// EDGE CASE 1: Zero vector
TEST(VcrossTest, EdgeCase_ZeroVector) {
    real3<float> zero(0.0f, 0.0f, 0.0f);
    real3<float> v(1.0f, 2.0f, 3.0f);
    
    real3<float> cross_result1 = vcross(zero, v);
    real3<float> cross_result2 = vcross(v, zero);
    
    EXPECT_NEAR(vlength(cross_result1), 0.0f, 1e-6f) 
        << "Cross product with zero vector should be zero";
    EXPECT_NEAR(vlength(cross_result2), 0.0f, 1e-6f) 
        << "Cross product with zero vector should be zero";
}

// EDGE CASE 2: Orthogonal vectors
TEST(VcrossTest, EdgeCase_OrthogonalVectors) {
    real3<float> x(1.0f, 0.0f, 0.0f);
    real3<float> y(0.0f, 1.0f, 0.0f);
    
    real3<float> cross_result = vcross(x, y);
    
    // Should be perpendicular to both
    EXPECT_NEAR(vdot(cross_result, x), 0.0f, 1e-6f);
    EXPECT_NEAR(vdot(cross_result, y), 0.0f, 1e-6f);
    
    // Magnitude should be |x| * |y| = 1 * 1 = 1
    EXPECT_NEAR(vlength(cross_result), 1.0f, 1e-6f);
}

// EDGE CASE 3: Collinear vectors (parallel)
TEST(VcrossTest, EdgeCase_CollinearVectors) {
    real3<float> v1(2.0f, 4.0f, 6.0f);
    real3<float> v2(1.0f, 2.0f, 3.0f);  // v1 = 2 * v2
    
    real3<float> cross_result = vcross(v1, v2);
    
    EXPECT_NEAR(cross_result[0], 0.0f, 1e-5f) << "Collinear vectors should have zero cross product";
    EXPECT_NEAR(cross_result[1], 0.0f, 1e-5f);
    EXPECT_NEAR(cross_result[2], 0.0f, 1e-5f);
}

// EDGE CASE 4: Collinear vectors (anti-parallel)
TEST(VcrossTest, EdgeCase_AntiParallelVectors) {
    real3<float> v1(2.0f, 4.0f, 6.0f);
    real3<float> v2(-1.0f, -2.0f, -3.0f);  // v1 = -2 * v2
    
    real3<float> cross_result = vcross(v1, v2);
    
    EXPECT_NEAR(cross_result[0], 0.0f, 1e-5f) << "Anti-parallel vectors should have zero cross product";
    EXPECT_NEAR(cross_result[1], 0.0f, 1e-5f);
    EXPECT_NEAR(cross_result[2], 0.0f, 1e-5f);
}

// EDGE CASE 5: Large magnitude vectors
TEST(VcrossTest, EdgeCase_LargeMagnitude) {
    // Use large but not overflow-prone values
    real3<float> large1(1e6f, 2e6f, 3e6f);
    real3<float> large2(4e6f, 5e6f, 6e6f);
    
    real3<float> cross_result = vcross(large1, large2);
    
    // Should not overflow to infinity
    EXPECT_FALSE(std::isinf(cross_result[0])) << "Large vector cross product should not overflow";
    EXPECT_FALSE(std::isinf(cross_result[1]));
    EXPECT_FALSE(std::isinf(cross_result[2]));
    EXPECT_FALSE(std::isnan(cross_result[0])) << "Large vector cross product should not be NaN";
    EXPECT_FALSE(std::isnan(cross_result[1]));
    EXPECT_FALSE(std::isnan(cross_result[2]));
    
    // Should still be perpendicular (with appropriate tolerance for large numbers)
    float dot1 = vdot(cross_result, large1);
    float dot2 = vdot(cross_result, large2);
    float tolerance = vlength(cross_result) * vlength(large1) * 1e-5f;
    EXPECT_NEAR(dot1, 0.0f, tolerance) << "Should be perpendicular to first vector";
    EXPECT_NEAR(dot2, 0.0f, tolerance) << "Should be perpendicular to second vector";
}

// EDGE CASE 6: Unit vectors
TEST(VcrossTest, EdgeCase_UnitVectors) {
    real3<float> u1(1.0f, 0.0f, 0.0f);
    real3<float> u2(0.0f, 1.0f, 0.0f);
    
    real3<float> cross_result = vcross(u1, u2);
    
    // Cross product of orthogonal unit vectors should be unit vector
    EXPECT_NEAR(vlength(cross_result), 1.0f, 1e-6f) 
        << "Cross product of orthogonal unit vectors should have unit length";
}

// EDGE CASE 7: Mixed positive/negative components
TEST(VcrossTest, EdgeCase_MixedComponents) {
    real3<float> mixed1(1.0f, -2.0f, 3.0f);
    real3<float> mixed2(-4.0f, 5.0f, -6.0f);
    
    real3<float> cross_result = vcross(mixed1, mixed2);
    
    // Should still be perpendicular
    EXPECT_NEAR(vdot(cross_result, mixed1), 0.0f, 1e-4f);
    EXPECT_NEAR(vdot(cross_result, mixed2), 0.0f, 1e-4f);
    
    // Should satisfy anti-commutativity
    real3<float> cross_reverse = vcross(mixed2, mixed1);
    EXPECT_NEAR(cross_result[0], -cross_reverse[0], 1e-5f);
    EXPECT_NEAR(cross_result[1], -cross_reverse[1], 1e-5f);
    EXPECT_NEAR(cross_result[2], -cross_reverse[2], 1e-5f);
}

// ============================================================================
// KNOWN VALUE TESTS: Hand-calculated results
// ============================================================================

TEST(VcrossTest, KnownValue_BasisVectors) {
    real3<float> x(1.0f, 0.0f, 0.0f);
    real3<float> y(0.0f, 1.0f, 0.0f);
    
    real3<float> cross_result = vcross(x, y);
    
    EXPECT_FLOAT_EQ(cross_result[0], 0.0f);
    EXPECT_FLOAT_EQ(cross_result[1], 0.0f);
    EXPECT_FLOAT_EQ(cross_result[2], 1.0f);
}

TEST(VcrossTest, KnownValue_General) {
    real3<float> a(1.0f, 2.0f, 3.0f);
    real3<float> b(4.0f, 5.0f, 6.0f);
    
    // cross(a, b) = (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4)
    //             = (12 - 15, 12 - 6, 5 - 8)
    //             = (-3, 6, -3)
    real3<float> cross_result = vcross(a, b);
    
    EXPECT_FLOAT_EQ(cross_result[0], -3.0f);
    EXPECT_FLOAT_EQ(cross_result[1], 6.0f);
    EXPECT_FLOAT_EQ(cross_result[2], -3.0f);
}
