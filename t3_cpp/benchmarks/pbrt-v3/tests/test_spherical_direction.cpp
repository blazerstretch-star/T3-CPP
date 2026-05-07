#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Result always has unit length (when sin²θ + cos²θ = 1)
TEST(SphericalDirectionTest, UnitLength) {
    Float angles[] = {0.0f, Pi/6, Pi/4, Pi/3, Pi/2};
    
    for (Float theta : angles) {
        Float sinTheta = std::sin(theta);
        Float cosTheta = std::cos(theta);
        Float phi = Pi / 4;
        
        Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 2: Z-axis direction (theta = 0)
TEST(SphericalDirectionTest, ZAxis) {
    Float sinTheta = 0.0f;
    Float cosTheta = 1.0f;
    Float phi = 0.0f;
    
    Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
}

// Property 3: X-axis direction (theta = 90°, phi = 0)
TEST(SphericalDirectionTest, XAxis) {
    Float sinTheta = 1.0f;
    Float cosTheta = 0.0f;
    Float phi = 0.0f;
    
    Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 4: Y-axis direction (theta = 90°, phi = 90°)
TEST(SphericalDirectionTest, YAxis) {
    Float sinTheta = 1.0f;
    Float cosTheta = 0.0f;
    Float phi = Pi / 2;
    
    Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 1.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 5: Z-component equals cosTheta
TEST(SphericalDirectionTest, ZComponentEqualsCosTheta) {
    Float cosTheta_values[] = {0.0f, 0.5f, 0.707f, 1.0f};
    
    for (Float cosTheta : cosTheta_values) {
        Float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);
        Float phi = Pi / 4;
        
        Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
        EXPECT_NEAR(result.z, cosTheta, 1e-6f);
    }
}

// Property 6: Phi rotation around z-axis
TEST(SphericalDirectionTest, PhiRotation) {
    Float sinTheta = 0.6f;
    Float cosTheta = 0.8f;
    
    // At phi = 0, should be in xz-plane with positive x
    Vector3f result1 = SphericalDirection(sinTheta, cosTheta, 0.0f);
    EXPECT_GT(result1.x, 0.0f);
    EXPECT_NEAR(result1.y, 0.0f, 1e-6f);
    
    // At phi = π/2, should be in yz-plane with positive y
    Vector3f result2 = SphericalDirection(sinTheta, cosTheta, Pi / 2);
    EXPECT_NEAR(result2.x, 0.0f, 1e-6f);
    EXPECT_GT(result2.y, 0.0f);
    
    // Both should have same z
    EXPECT_NEAR(result1.z, result2.z, 1e-6f);
}

// Property 7: 45-degree angle
TEST(SphericalDirectionTest, FortyFiveDegrees) {
    Float angle = Pi / 4;
    Float sinTheta = std::sin(angle);
    Float cosTheta = std::cos(angle);
    Float phi = 0.0f;
    
    Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
    
    // At 45°, x and z should be equal
    EXPECT_NEAR(result.x, result.z, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 8: Opposite directions
TEST(SphericalDirectionTest, OppositeDirections) {
    Float sinTheta = 0.6f;
    Float cosTheta = 0.8f;
    Float phi = 0.0f;
    
    Vector3f result1 = SphericalDirection(sinTheta, cosTheta, phi);
    Vector3f result2 = SphericalDirection(sinTheta, cosTheta, phi + Pi);
    
    // Should be opposite in x-y plane, same z
    EXPECT_NEAR(result1.x, -result2.x, 1e-5f);
    EXPECT_NEAR(result1.y, -result2.y, 1e-5f);
    EXPECT_NEAR(result1.z, result2.z, 1e-6f);
}

// Property 9: Consistency with trigonometric identity
TEST(SphericalDirectionTest, TrigonometricIdentity) {
    Float theta = Pi / 3;
    Float sinTheta = std::sin(theta);
    Float cosTheta = std::cos(theta);
    Float phi = Pi / 6;
    
    Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
    
    // x² + y² + z² = 1
    Float sum_squares = result.x * result.x + result.y * result.y + result.z * result.z;
    EXPECT_NEAR(sum_squares, 1.0f, 1e-6f);
    
    // x² + y² = sin²θ
    Float xy_sum = result.x * result.x + result.y * result.y;
    EXPECT_NEAR(xy_sum, sinTheta * sinTheta, 1e-6f);
}

// Property 10: Full rotation coverage
TEST(SphericalDirectionTest, FullRotation) {
    Float sinTheta = 0.6f;
    Float cosTheta = 0.8f;
    
    bool has_positive_x = false;
    bool has_negative_x = false;
    bool has_positive_y = false;
    bool has_negative_y = false;
    
    for (Float phi = 0.0f; phi < 2 * Pi; phi += Pi / 4) {
        Vector3f result = SphericalDirection(sinTheta, cosTheta, phi);
        
        if (result.x > 0.1f) has_positive_x = true;
        if (result.x < -0.1f) has_negative_x = true;
        if (result.y > 0.1f) has_positive_y = true;
        if (result.y < -0.1f) has_negative_y = true;
    }
    
    EXPECT_TRUE(has_positive_x);
    EXPECT_TRUE(has_negative_x);
    EXPECT_TRUE(has_positive_y);
    EXPECT_TRUE(has_negative_y);
}
