#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

using namespace nanort;

// ============================================================================
// PROPERTY-BASED TESTS: Verify mathematical invariants, not specific outputs
// ============================================================================

// PROPERTY 1: Unit length - normalized vector should have length 1
TEST(VnormalizeTest, Property_UnitLength) {
    // Test with various non-zero vectors
    real3<float> v1(3.0f, 4.0f, 0.0f);
    real3<float> n1 = vnormalize(v1);
    EXPECT_NEAR(vlength(n1), 1.0f, 1e-5f) << "Normalized vector should have unit length";
    
    real3<float> v2(1.0f, 2.0f, 3.0f);
    real3<float> n2 = vnormalize(v2);
    EXPECT_NEAR(vlength(n2), 1.0f, 1e-5f) << "Normalized vector should have unit length";
    
    real3<float> v3(-5.0f, 12.0f, -13.0f);
    real3<float> n3 = vnormalize(v3);
    EXPECT_NEAR(vlength(n3), 1.0f, 1e-5f) << "Normalized vector should have unit length";
}

// PROPERTY 2: Direction preservation - normalized vector should be parallel to original
TEST(VnormalizeTest, Property_DirectionPreservation) {
    real3<float> v(3.0f, 4.0f, 5.0f);
    real3<float> n = vnormalize(v);
    
    // Cross product of parallel vectors should be zero
    real3<float> cross_result = vcross(v, n);
    EXPECT_NEAR(vlength(cross_result), 0.0f, 1e-5f) 
        << "Normalized vector should be parallel to original";
    
    // Dot product should equal product of lengths (for parallel vectors)
    float dot_result = vdot(v, n);
    float expected_dot = vlength(v) * vlength(n);
    EXPECT_NEAR(std::abs(dot_result), expected_dot, 1e-5f)
        << "Normalized vector should preserve direction";
}

// PROPERTY 3: Idempotence - normalizing twice should give same result
TEST(VnormalizeTest, Property_Idempotence) {
    real3<float> v(7.0f, 8.0f, 9.0f);
    real3<float> n1 = vnormalize(v);
    real3<float> n2 = vnormalize(n1);
    
    EXPECT_NEAR(n1[0], n2[0], 1e-5f) << "normalize(normalize(v)) should equal normalize(v)";
    EXPECT_NEAR(n1[1], n2[1], 1e-5f) << "normalize(normalize(v)) should equal normalize(v)";
    EXPECT_NEAR(n1[2], n2[2], 1e-5f) << "normalize(normalize(v)) should equal normalize(v)";
    EXPECT_NEAR(vlength(n2), 1.0f, 1e-5f) << "Double normalization should still have unit length";
}

// PROPERTY 4: Scale invariance - normalize(k*v) should equal normalize(v) for k > 0
TEST(VnormalizeTest, Property_ScaleInvariance_Positive) {
    real3<float> v(1.0f, 2.0f, 3.0f);
    real3<float> n1 = vnormalize(v);
    
    // Test with various positive scales
    real3<float> v2 = 5.0f * v;
    real3<float> n2 = vnormalize(v2);
    
    EXPECT_NEAR(n1[0], n2[0], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
    EXPECT_NEAR(n1[1], n2[1], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
    EXPECT_NEAR(n1[2], n2[2], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
    
    real3<float> v3 = 0.1f * v;
    real3<float> n3 = vnormalize(v3);
    
    EXPECT_NEAR(n1[0], n3[0], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
    EXPECT_NEAR(n1[1], n3[1], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
    EXPECT_NEAR(n1[2], n3[2], 1e-5f) << "normalize(k*v) should equal normalize(v) for k > 0";
}

// PROPERTY 5: Scale invariance - normalize(-v) should equal -normalize(v)
TEST(VnormalizeTest, Property_ScaleInvariance_Negative) {
    real3<float> v(2.0f, 3.0f, 4.0f);
    real3<float> n1 = vnormalize(v);
    real3<float> n2 = vnormalize(vneg(v));
    
    EXPECT_NEAR(n1[0], -n2[0], 1e-5f) << "normalize(-v) should equal -normalize(v)";
    EXPECT_NEAR(n1[1], -n2[1], 1e-5f) << "normalize(-v) should equal -normalize(v)";
    EXPECT_NEAR(n1[2], -n2[2], 1e-5f) << "normalize(-v) should equal -normalize(v)";
}

// ============================================================================
// EDGE CASE TESTS: Boundary conditions and special values
// ============================================================================

// EDGE CASE 1: Zero vector - should not produce NaN or Inf
TEST(VnormalizeTest, EdgeCase_ZeroVector) {
    real3<float> zero(0.0f, 0.0f, 0.0f);
    real3<float> result = vnormalize(zero);
    
    // Should not produce NaN or Inf
    EXPECT_FALSE(std::isnan(result[0])) << "Zero vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[1])) << "Zero vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[2])) << "Zero vector normalization should not produce NaN";
    EXPECT_FALSE(std::isinf(result[0])) << "Zero vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[1])) << "Zero vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[2])) << "Zero vector normalization should not produce Inf";
    
    // Common implementations: return zero or return unchanged
    // Both are acceptable, so we just verify no NaN/Inf
}

// EDGE CASE 2: Already normalized vector
TEST(VnormalizeTest, EdgeCase_AlreadyNormalized) {
    real3<float> unit(1.0f, 0.0f, 0.0f);
    real3<float> result = vnormalize(unit);
    
    EXPECT_NEAR(result[0], 1.0f, 1e-6f) << "Already normalized vector should remain unchanged";
    EXPECT_NEAR(result[1], 0.0f, 1e-6f) << "Already normalized vector should remain unchanged";
    EXPECT_NEAR(result[2], 0.0f, 1e-6f) << "Already normalized vector should remain unchanged";
    EXPECT_NEAR(vlength(result), 1.0f, 1e-6f) << "Should maintain unit length";
}

// EDGE CASE 3: Very small vector (near epsilon)
TEST(VnormalizeTest, EdgeCase_VerySmallVector) {
    float epsilon = std::numeric_limits<float>::epsilon();
    real3<float> tiny(epsilon * 10.0f, epsilon * 10.0f, epsilon * 10.0f);
    real3<float> result = vnormalize(tiny);
    
    // Should not produce NaN or Inf
    EXPECT_FALSE(std::isnan(result[0])) << "Small vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[1])) << "Small vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[2])) << "Small vector normalization should not produce NaN";
    EXPECT_FALSE(std::isinf(result[0])) << "Small vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[1])) << "Small vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[2])) << "Small vector normalization should not produce Inf";
    
    // If not treated as zero, should have unit length
    float len = vlength(result);
    if (len > 0.1f) {  // If implementation normalized it
        EXPECT_NEAR(len, 1.0f, 1e-5f) << "Normalized small vector should have unit length";
    }
}

// EDGE CASE 4: Very large vector
TEST(VnormalizeTest, EdgeCase_VeryLargeVector) {
    // Use large but not overflow-prone values
    real3<float> large(1e6f, 1e6f, 1e6f);
    real3<float> result = vnormalize(large);
    
    // Should not produce NaN or Inf
    EXPECT_FALSE(std::isnan(result[0])) << "Large vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[1])) << "Large vector normalization should not produce NaN";
    EXPECT_FALSE(std::isnan(result[2])) << "Large vector normalization should not produce NaN";
    EXPECT_FALSE(std::isinf(result[0])) << "Large vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[1])) << "Large vector normalization should not produce Inf";
    EXPECT_FALSE(std::isinf(result[2])) << "Large vector normalization should not produce Inf";
    
    EXPECT_NEAR(vlength(result), 1.0f, 1e-5f) << "Normalized large vector should have unit length";
}

// EDGE CASE 5: Vector with mixed positive/negative components
TEST(VnormalizeTest, EdgeCase_MixedComponents) {
    real3<float> mixed(-3.0f, 4.0f, -12.0f);
    real3<float> result = vnormalize(mixed);
    
    EXPECT_NEAR(vlength(result), 1.0f, 1e-5f) << "Normalized mixed vector should have unit length";
    
    // Check sign preservation
    EXPECT_LT(result[0], 0.0f) << "Negative component should remain negative";
    EXPECT_GT(result[1], 0.0f) << "Positive component should remain positive";
    EXPECT_LT(result[2], 0.0f) << "Negative component should remain negative";
}

// EDGE CASE 6: Vector with one dominant component
TEST(VnormalizeTest, EdgeCase_DominantComponent) {
    real3<float> dominant(1000.0f, 0.001f, 0.001f);
    real3<float> result = vnormalize(dominant);
    
    EXPECT_NEAR(vlength(result), 1.0f, 1e-5f) << "Normalized vector should have unit length";
    EXPECT_NEAR(result[0], 1.0f, 1e-3f) << "Dominant component should be close to 1";
    EXPECT_NEAR(result[1], 0.0f, 1e-3f) << "Small component should be close to 0";
    EXPECT_NEAR(result[2], 0.0f, 1e-3f) << "Small component should be close to 0";
}

// ============================================================================
// KNOWN VALUE TESTS: Verify specific cases with hand-calculated results
// ============================================================================

TEST(VnormalizeTest, KnownValue_3_4_0) {
    real3<float> v(3.0f, 4.0f, 0.0f);
    real3<float> result = vnormalize(v);
    
    // Length = sqrt(9 + 16) = 5
    // Normalized = (3/5, 4/5, 0) = (0.6, 0.8, 0)
    EXPECT_NEAR(result[0], 0.6f, 1e-6f);
    EXPECT_NEAR(result[1], 0.8f, 1e-6f);
    EXPECT_NEAR(result[2], 0.0f, 1e-6f);
    EXPECT_NEAR(vlength(result), 1.0f, 1e-6f);
}

TEST(VnormalizeTest, KnownValue_1_1_1) {
    real3<float> v(1.0f, 1.0f, 1.0f);
    real3<float> result = vnormalize(v);
    
    // Length = sqrt(3)
    // Normalized = (1/sqrt(3), 1/sqrt(3), 1/sqrt(3))
    float expected = 1.0f / std::sqrt(3.0f);
    EXPECT_NEAR(result[0], expected, 1e-6f);
    EXPECT_NEAR(result[1], expected, 1e-6f);
    EXPECT_NEAR(result[2], expected, 1e-6f);
    EXPECT_NEAR(vlength(result), 1.0f, 1e-6f);
}
