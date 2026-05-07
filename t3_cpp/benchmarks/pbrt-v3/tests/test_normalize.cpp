#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Result always has unit length
TEST(NormalizeTest, UnitLength) {
    Vector3f v1(3.0f, 4.0f, 0.0f);
    Vector3f result1 = Normalize(v1);
    EXPECT_NEAR(result1.Length(), 1.0f, 1e-6f);
    
    Vector3f v2(1.5f, 2.7f, -3.2f);
    Vector3f result2 = Normalize(v2);
    EXPECT_NEAR(result2.Length(), 1.0f, 1e-6f);
}

// Property 2: Direction is preserved (parallel to original)
TEST(NormalizeTest, DirectionPreserved) {
    Vector3f v(2.0f, 3.0f, 4.0f);
    Vector3f normalized = Normalize(v);
    
    // Cross product of parallel vectors is zero
    Vector3f cross = Cross(v, normalized);
    EXPECT_NEAR(cross.x, 0.0f, 1e-5f);
    EXPECT_NEAR(cross.y, 0.0f, 1e-5f);
    EXPECT_NEAR(cross.z, 0.0f, 1e-5f);
}

// Property 3: Normalizing a unit vector returns itself
TEST(NormalizeTest, AlreadyNormalized) {
    Vector3f v(1.0f, 0.0f, 0.0f);
    Vector3f result = Normalize(v);
    
    EXPECT_NEAR(result.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 4: Known result - 3-4-5 triangle
TEST(NormalizeTest, ThreeFourFive) {
    Vector3f v(3.0f, 4.0f, 0.0f);
    Vector3f result = Normalize(v);
    
    EXPECT_NEAR(result.x, 0.6f, 1e-6f);
    EXPECT_NEAR(result.y, 0.8f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 5: Idempotence - Normalize(Normalize(v)) = Normalize(v)
TEST(NormalizeTest, Idempotence) {
    Vector3f v(5.0f, -3.0f, 2.0f);
    Vector3f once = Normalize(v);
    Vector3f twice = Normalize(once);
    
    EXPECT_NEAR(once.x, twice.x, 1e-6f);
    EXPECT_NEAR(once.y, twice.y, 1e-6f);
    EXPECT_NEAR(once.z, twice.z, 1e-6f);
}

// Property 6: Scaling doesn't change normalized result
TEST(NormalizeTest, ScaleInvariance) {
    Vector3f v(1.0f, 2.0f, 3.0f);
    Vector3f v_scaled(5.0f, 10.0f, 15.0f);  // 5x scaled
    
    Vector3f norm1 = Normalize(v);
    Vector3f norm2 = Normalize(v_scaled);
    
    EXPECT_NEAR(norm1.x, norm2.x, 1e-6f);
    EXPECT_NEAR(norm1.y, norm2.y, 1e-6f);
    EXPECT_NEAR(norm1.z, norm2.z, 1e-6f);
}

// Property 7: Negative vector normalization
TEST(NormalizeTest, NegativeVector) {
    Vector3f v(2.0f, 3.0f, 4.0f);
    Vector3f neg_v(-2.0f, -3.0f, -4.0f);
    
    Vector3f norm_v = Normalize(v);
    Vector3f norm_neg = Normalize(neg_v);
    
    // Should be exact opposites
    EXPECT_NEAR(norm_v.x, -norm_neg.x, 1e-6f);
    EXPECT_NEAR(norm_v.y, -norm_neg.y, 1e-6f);
    EXPECT_NEAR(norm_v.z, -norm_neg.z, 1e-6f);
}

// Property 8: Standard basis vectors remain unchanged
TEST(NormalizeTest, BasisVectors) {
    Vector3f x_axis(1.0f, 0.0f, 0.0f);
    Vector3f y_axis(0.0f, 1.0f, 0.0f);
    Vector3f z_axis(0.0f, 0.0f, 1.0f);
    
    Vector3f norm_x = Normalize(x_axis);
    Vector3f norm_y = Normalize(y_axis);
    Vector3f norm_z = Normalize(z_axis);
    
    EXPECT_NEAR(norm_x.x, 1.0f, 1e-6f);
    EXPECT_NEAR(norm_y.y, 1.0f, 1e-6f);
    EXPECT_NEAR(norm_z.z, 1.0f, 1e-6f);
}

// Property 9: Small vectors
TEST(NormalizeTest, SmallVectors) {
    Vector3f v(0.001f, 0.002f, 0.003f);
    Vector3f result = Normalize(v);
    
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 10: Large vectors
TEST(NormalizeTest, LargeVectors) {
    Vector3f v(1000.0f, 2000.0f, 3000.0f);
    Vector3f result = Normalize(v);
    
    EXPECT_NEAR(result.Length(), 1.0f, 1e-5f);
    
    // Direction should still be preserved
    Float ratio_xy = v.x / v.y;
    Float result_ratio_xy = result.x / result.y;
    EXPECT_NEAR(ratio_xy, result_ratio_xy, 1e-5f);
}
