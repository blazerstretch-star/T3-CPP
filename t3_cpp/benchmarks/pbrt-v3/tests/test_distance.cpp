#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Distance is always non-negative
TEST(DistanceTest, NonNegative) {
    Point3f p1(1.0f, 2.0f, 3.0f);
    Point3f p2(4.0f, 5.0f, 6.0f);
    EXPECT_GE(Distance(p1, p2), 0.0f);
}

// Property 2: Distance from point to itself is zero
TEST(DistanceTest, SamePoint) {
    Point3f p(5.0f, -3.0f, 2.0f);
    EXPECT_NEAR(Distance(p, p), 0.0f, 1e-6f);
}

// Property 3: Symmetry - Distance(a,b) = Distance(b,a)
TEST(DistanceTest, Symmetry) {
    Point3f p1(1.5f, 2.7f, -3.2f);
    Point3f p2(4.1f, -1.8f, 5.3f);
    EXPECT_NEAR(Distance(p1, p2), Distance(p2, p1), 1e-6f);
}

// Property 4: Known result - 3-4-5 right triangle
TEST(DistanceTest, ThreeFourFive) {
    Point3f p1(0.0f, 0.0f, 0.0f);
    Point3f p2(3.0f, 4.0f, 0.0f);
    EXPECT_NEAR(Distance(p1, p2), 5.0f, 1e-6f);
}

// Property 5: Known result - unit distance along axes
TEST(DistanceTest, UnitDistanceAlongAxes) {
    Point3f origin(0.0f, 0.0f, 0.0f);
    Point3f x_unit(1.0f, 0.0f, 0.0f);
    Point3f y_unit(0.0f, 1.0f, 0.0f);
    Point3f z_unit(0.0f, 0.0f, 1.0f);
    
    EXPECT_NEAR(Distance(origin, x_unit), 1.0f, 1e-6f);
    EXPECT_NEAR(Distance(origin, y_unit), 1.0f, 1e-6f);
    EXPECT_NEAR(Distance(origin, z_unit), 1.0f, 1e-6f);
}

// Property 6: Triangle inequality - Distance(a,c) <= Distance(a,b) + Distance(b,c)
TEST(DistanceTest, TriangleInequality) {
    Point3f a(0.0f, 0.0f, 0.0f);
    Point3f b(1.0f, 1.0f, 1.0f);
    Point3f c(2.0f, 2.0f, 2.0f);
    
    Float d_ac = Distance(a, c);
    Float d_ab = Distance(a, b);
    Float d_bc = Distance(b, c);
    
    EXPECT_LE(d_ac, d_ab + d_bc + 1e-5f);
}

// Property 7: Scaling property
TEST(DistanceTest, ScalingProperty) {
    Point3f p1(1.0f, 2.0f, 3.0f);
    Point3f p2(4.0f, 5.0f, 6.0f);
    
    Float k = 2.0f;
    Point3f p1_scaled(k * p1.x, k * p1.y, k * p1.z);
    Point3f p2_scaled(k * p2.x, k * p2.y, k * p2.z);
    
    Float dist_original = Distance(p1, p2);
    Float dist_scaled = Distance(p1_scaled, p2_scaled);
    
    EXPECT_NEAR(dist_scaled, k * dist_original, 1e-5f);
}

// Property 8: 3D Pythagorean theorem
TEST(DistanceTest, ThreeDimensionalPythagorean) {
    Point3f p1(1.0f, 2.0f, 3.0f);
    Point3f p2(4.0f, 6.0f, 8.0f);
    
    Float dx = 3.0f;  // 4 - 1
    Float dy = 4.0f;  // 6 - 2
    Float dz = 5.0f;  // 8 - 3
    Float expected = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    EXPECT_NEAR(Distance(p1, p2), expected, 1e-5f);
}

// Property 9: Distance along diagonal
TEST(DistanceTest, DiagonalDistance) {
    Point3f p1(0.0f, 0.0f, 0.0f);
    Point3f p2(1.0f, 1.0f, 1.0f);
    
    Float expected = std::sqrt(3.0f);  // sqrt(1^2 + 1^2 + 1^2)
    EXPECT_NEAR(Distance(p1, p2), expected, 1e-6f);
}

// Property 10: Large coordinates
TEST(DistanceTest, LargeCoordinates) {
    Point3f p1(1000.0f, 2000.0f, 3000.0f);
    Point3f p2(1003.0f, 2004.0f, 3000.0f);
    
    // Distance should be 5 (3-4-5 triangle)
    EXPECT_NEAR(Distance(p1, p2), 5.0f, 1e-4f);
}
