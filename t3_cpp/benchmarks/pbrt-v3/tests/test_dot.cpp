#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Commutativity - Dot(a,b) = Dot(b,a)
TEST(DotTest, Commutativity) {
    Vector3f v1(1.5f, 2.3f, -3.7f);
    Vector3f v2(4.2f, -1.8f, 5.1f);
    EXPECT_NEAR(Dot(v1, v2), Dot(v2, v1), 1e-5f);
}

// Property 2: Perpendicular vectors have dot product = 0
TEST(DotTest, PerpendicularVectors) {
    Vector3f v1(1.0f, 0.0f, 0.0f);
    Vector3f v2(0.0f, 1.0f, 0.0f);
    EXPECT_NEAR(Dot(v1, v2), 0.0f, 1e-6f);
    
    Vector3f v3(0.0f, 0.0f, 1.0f);
    EXPECT_NEAR(Dot(v1, v3), 0.0f, 1e-6f);
    EXPECT_NEAR(Dot(v2, v3), 0.0f, 1e-6f);
}

// Property 3: Parallel vectors - Dot(a, k*a) = k * |a|^2
TEST(DotTest, ParallelVectors) {
    Vector3f v(3.0f, 4.0f, 5.0f);
    Float k = 2.5f;
    Vector3f v_scaled(k * v.x, k * v.y, k * v.z);
    
    Float dot_result = Dot(v, v_scaled);
    Float expected = k * v.LengthSquared();
    EXPECT_NEAR(dot_result, expected, 1e-4f);
}

// Property 4: Self dot product equals squared length
TEST(DotTest, SelfDotProduct) {
    Vector3f v(2.5f, -3.7f, 4.2f);
    Float dot_self = Dot(v, v);
    Float length_sq = v.LengthSquared();
    EXPECT_NEAR(dot_self, length_sq, 1e-5f);
}

// Property 5: Distributivity - Dot(a, b+c) = Dot(a,b) + Dot(a,c)
TEST(DotTest, Distributivity) {
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);
    Vector3f c(7.0f, 8.0f, 9.0f);
    Vector3f b_plus_c(b.x + c.x, b.y + c.y, b.z + c.z);
    
    Float left = Dot(a, b_plus_c);
    Float right = Dot(a, b) + Dot(a, c);
    EXPECT_NEAR(left, right, 1e-5f);
}

// Property 6: Zero vector
TEST(DotTest, ZeroVector) {
    Vector3f v(5.0f, -3.0f, 2.0f);
    Vector3f zero(0.0f, 0.0f, 0.0f);
    EXPECT_NEAR(Dot(v, zero), 0.0f, 1e-6f);
}

// Property 7: Negative vectors
TEST(DotTest, NegativeVectors) {
    Vector3f v(2.0f, 3.0f, 4.0f);
    Vector3f neg_v(-v.x, -v.y, -v.z);
    Float dot_result = Dot(v, neg_v);
    EXPECT_NEAR(dot_result, -v.LengthSquared(), 1e-5f);
}

// Property 8: Known result verification
TEST(DotTest, KnownResults) {
    Vector3f v1(1.0f, 2.0f, 3.0f);
    Vector3f v2(4.0f, 5.0f, 6.0f);
    // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    EXPECT_NEAR(Dot(v1, v2), 32.0f, 1e-5f);
}

// Property 9: Cauchy-Schwarz inequality - |Dot(a,b)| <= |a| * |b|
TEST(DotTest, CauchySchwarzInequality) {
    Vector3f v1(3.0f, -2.0f, 5.0f);
    Vector3f v2(1.0f, 4.0f, -2.0f);
    
    Float dot_abs = std::abs(Dot(v1, v2));
    Float product = v1.Length() * v2.Length();
    EXPECT_LE(dot_abs, product + 1e-5f);
}

// Property 10: Small values
TEST(DotTest, SmallValues) {
    Vector3f v1(0.001f, 0.002f, 0.003f);
    Vector3f v2(0.004f, 0.005f, 0.006f);
    Float expected = 0.001f*0.004f + 0.002f*0.005f + 0.003f*0.006f;
    EXPECT_NEAR(Dot(v1, v2), expected, 1e-8f);
}
