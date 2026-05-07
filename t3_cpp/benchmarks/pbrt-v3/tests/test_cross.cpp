#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Anti-commutativity - Cross(a,b) = -Cross(b,a)
TEST(CrossTest, AntiCommutativity) {
    Vector3f v1(1.5f, 2.3f, -3.7f);
    Vector3f v2(4.2f, -1.8f, 5.1f);
    Vector3f cross1 = Cross(v1, v2);
    Vector3f cross2 = Cross(v2, v1);
    
    EXPECT_NEAR(cross1.x, -cross2.x, 1e-5f);
    EXPECT_NEAR(cross1.y, -cross2.y, 1e-5f);
    EXPECT_NEAR(cross1.z, -cross2.z, 1e-5f);
}

// Property 2: Result is perpendicular to both input vectors
TEST(CrossTest, Perpendicularity) {
    Vector3f v1(1.0f, 2.0f, 3.0f);
    Vector3f v2(4.0f, 5.0f, 6.0f);
    Vector3f result = Cross(v1, v2);
    
    EXPECT_NEAR(Dot(result, v1), 0.0f, 1e-5f);
    EXPECT_NEAR(Dot(result, v2), 0.0f, 1e-5f);
}

// Property 3: Cross product of parallel vectors is zero
TEST(CrossTest, ParallelVectors) {
    Vector3f v(2.0f, 3.0f, 4.0f);
    Float k = 3.5f;
    Vector3f v_scaled(k * v.x, k * v.y, k * v.z);
    Vector3f result = Cross(v, v_scaled);
    
    EXPECT_NEAR(result.x, 0.0f, 1e-5f);
    EXPECT_NEAR(result.y, 0.0f, 1e-5f);
    EXPECT_NEAR(result.z, 0.0f, 1e-5f);
}

// Property 4: Cross product with itself is zero
TEST(CrossTest, SelfCross) {
    Vector3f v(5.0f, -3.0f, 2.0f);
    Vector3f result = Cross(v, v);
    
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 5: Standard basis vectors
TEST(CrossTest, StandardBasisVectors) {
    Vector3f i(1.0f, 0.0f, 0.0f);
    Vector3f j(0.0f, 1.0f, 0.0f);
    Vector3f k(0.0f, 0.0f, 1.0f);
    
    // i × j = k
    Vector3f result1 = Cross(i, j);
    EXPECT_NEAR(result1.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result1.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result1.z, 1.0f, 1e-6f);
    
    // j × k = i
    Vector3f result2 = Cross(j, k);
    EXPECT_NEAR(result2.x, 1.0f, 1e-6f);
    EXPECT_NEAR(result2.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result2.z, 0.0f, 1e-6f);
    
    // k × i = j
    Vector3f result3 = Cross(k, i);
    EXPECT_NEAR(result3.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result3.y, 1.0f, 1e-6f);
    EXPECT_NEAR(result3.z, 0.0f, 1e-6f);
}

// Property 6: Magnitude relationship - |a × b| = |a| * |b| * sin(θ)
TEST(CrossTest, MagnitudeForPerpendicularVectors) {
    Vector3f v1(3.0f, 0.0f, 0.0f);
    Vector3f v2(0.0f, 4.0f, 0.0f);
    Vector3f result = Cross(v1, v2);
    
    // For perpendicular vectors, |a × b| = |a| * |b|
    Float expected_magnitude = v1.Length() * v2.Length();
    EXPECT_NEAR(result.Length(), expected_magnitude, 1e-5f);
}

// Property 7: Distributivity - a × (b + c) = (a × b) + (a × c)
TEST(CrossTest, Distributivity) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);
    Vector3f c(7.0f, 8.0f, 9.0f);
    Vector3f b_plus_c(b.x + c.x, b.y + c.y, b.z + c.z);
    
    Vector3f left = Cross(a, b_plus_c);
    Vector3f cross_ab = Cross(a, b);
    Vector3f cross_ac = Cross(a, c);
    Vector3f right(cross_ab.x + cross_ac.x, cross_ab.y + cross_ac.y, cross_ab.z + cross_ac.z);
    
    EXPECT_NEAR(left.x, right.x, 1e-5f);
    EXPECT_NEAR(left.y, right.y, 1e-5f);
    EXPECT_NEAR(left.z, right.z, 1e-5f);
}

// Property 8: Cross product with zero vector
TEST(CrossTest, ZeroVector) {
    Vector3f v(5.0f, -3.0f, 2.0f);
    Vector3f zero(0.0f, 0.0f, 0.0f);
    Vector3f result = Cross(v, zero);
    
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
}

// Property 9: Known result verification
TEST(CrossTest, KnownResult) {
    Vector3f v1(1.0f, 2.0f, 3.0f);
    Vector3f v2(4.0f, 5.0f, 6.0f);
    Vector3f result = Cross(v1, v2);
    
    // (2*6 - 3*5, 3*4 - 1*6, 1*5 - 2*4) = (-3, 6, -3)
    EXPECT_NEAR(result.x, -3.0f, 1e-5f);
    EXPECT_NEAR(result.y, 6.0f, 1e-5f);
    EXPECT_NEAR(result.z, -3.0f, 1e-5f);
}

// Property 10: Jacobi identity - a × (b × c) + b × (c × a) + c × (a × b) = 0
TEST(CrossTest, JacobiIdentity) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);
    Vector3f c(7.0f, 8.0f, 9.0f);
    
    Vector3f bc = Cross(b, c);
    Vector3f ca = Cross(c, a);
    Vector3f ab = Cross(a, b);
    
    Vector3f term1 = Cross(a, bc);
    Vector3f term2 = Cross(b, ca);
    Vector3f term3 = Cross(c, ab);
    
    Vector3f sum(term1.x + term2.x + term3.x,
                 term1.y + term2.y + term3.y,
                 term1.z + term2.z + term3.z);
    
    EXPECT_NEAR(sum.x, 0.0f, 1e-4f);
    EXPECT_NEAR(sum.y, 0.0f, 1e-4f);
    EXPECT_NEAR(sum.z, 0.0f, 1e-4f);
}
