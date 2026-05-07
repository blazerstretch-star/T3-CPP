#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: All three vectors are mutually orthogonal
TEST(CoordinateSystemTest, MutualOrthogonality) {
    Vector3f v1(1.0f, 2.0f, 3.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-5f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-5f);
    EXPECT_NEAR(Dot(v2, v3), 0.0f, 1e-5f);
}

// Property 2: v2 is unit length (v3 length = |v1| since v3 = Cross(v1, v2))
TEST(CoordinateSystemTest, UnitVectors) {
    Vector3f v1(5.0f, -3.0f, 2.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    // v3 = Cross(v1, v2), so |v3| = |v1| * |v2| * sin(90°) = |v1|
    EXPECT_NEAR(v3.Length(), v1.Length(), 1e-5f);
}

// Property 3: Right-handed coordinate system - v3 = v1 × v2
TEST(CoordinateSystemTest, RightHandedness) {
    Vector3f v1(0.0f, 0.0f, 1.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    Vector3f cross_result = Cross(v1, v2);
    EXPECT_NEAR(cross_result.x, v3.x, 1e-5f);
    EXPECT_NEAR(cross_result.y, v3.y, 1e-5f);
    EXPECT_NEAR(cross_result.z, v3.z, 1e-5f);
}

// Property 4: Works with standard basis vectors
TEST(CoordinateSystemTest, StandardBasisX) {
    Vector3f v1(1.0f, 0.0f, 0.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    // v2 and v3 should be perpendicular to x-axis
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-6f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-6f);
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(v3.Length(), 1.0f, 1e-6f);
}

// Property 5: Works with standard basis vectors (Y-axis)
TEST(CoordinateSystemTest, StandardBasisY) {
    Vector3f v1(0.0f, 1.0f, 0.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-6f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-6f);
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(v3.Length(), 1.0f, 1e-6f);
}

// Property 6: Works with standard basis vectors (Z-axis)
TEST(CoordinateSystemTest, StandardBasisZ) {
    Vector3f v1(0.0f, 0.0f, 1.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-6f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-6f);
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(v3.Length(), 1.0f, 1e-6f);
}

// Property 7: Orthogonal basis (v2 is unit, v3 length = |v1|)
TEST(CoordinateSystemTest, OrthonormalBasis) {
    Vector3f v1(1.0f, 1.0f, 1.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    // v2 is unit length
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    // v3 length equals |v1|
    EXPECT_NEAR(v3.Length(), v1.Length(), 1e-5f);
    
    // They're mutually perpendicular
    EXPECT_NEAR(Dot(v2, v3), 0.0f, 1e-5f);
}

// Property 8: Diagonal vector
TEST(CoordinateSystemTest, DiagonalVector) {
    Vector3f v1(1.0f, 1.0f, 1.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    // All orthogonality conditions
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-5f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-5f);
    EXPECT_NEAR(Dot(v2, v3), 0.0f, 1e-5f);
}

// Property 9: Arbitrary vector
TEST(CoordinateSystemTest, ArbitraryVector) {
    Vector3f v1(3.7f, -2.1f, 5.4f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-4f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-4f);
    EXPECT_NEAR(Dot(v2, v3), 0.0f, 1e-5f);
    EXPECT_NEAR(v2.Length(), 1.0f, 1e-6f);
    // v3 length equals |v1|
    EXPECT_NEAR(v3.Length(), v1.Length(), 1e-5f);
}

// Property 10: Triple scalar product (v1 · (v2 × v3)) should be non-zero
TEST(CoordinateSystemTest, NonDegenerateSystem) {
    Vector3f v1(2.0f, 3.0f, 4.0f);
    Vector3f v2, v3;
    CoordinateSystem(v1, &v2, &v3);
    
    Vector3f cross_v2_v3 = Cross(v2, v3);
    Float triple_product = Dot(v1, cross_v2_v3);
    
    // Should be non-zero (actually should be close to |v1| for orthonormal system)
    EXPECT_GT(std::abs(triple_product), 0.1f);
}
