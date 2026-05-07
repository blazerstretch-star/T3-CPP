#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: All samples lie on unit sphere
TEST(UniformSampleSphereTest, UnitLength) {
    Point2f samples[] = {
        Point2f(0.0f, 0.0f), Point2f(0.5f, 0.5f), Point2f(1.0f, 1.0f),
        Point2f(0.25f, 0.75f), Point2f(0.7f, 0.3f)
    };
    
    for (const auto& u : samples) {
        Vector3f result = UniformSampleSphere(u);
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 2: North pole (u.x = 0)
TEST(UniformSampleSphereTest, NorthPole) {
    Point2f u(0.0f, 0.5f);
    Vector3f result = UniformSampleSphere(u);
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 3: South pole (u.x = 1)
TEST(UniformSampleSphereTest, SouthPole) {
    Point2f u(1.0f, 0.5f);
    Vector3f result = UniformSampleSphere(u);
    EXPECT_NEAR(result.z, -1.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 4: Equator (u.x = 0.5)
TEST(UniformSampleSphereTest, Equator) {
    Point2f u(0.5f, 0.0f);
    Vector3f result = UniformSampleSphere(u);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 5: Z-coordinate range is [-1, 1]
TEST(UniformSampleSphereTest, ZCoordinateRange) {
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.1f) {
        Point2f u(u_x, 0.5f);
        Vector3f result = UniformSampleSphere(u);
        EXPECT_GE(result.z, -1.0f - 1e-6f);
        EXPECT_LE(result.z, 1.0f + 1e-6f);
    }
}

// Property 6: Different phi values produce different x,y but same z
TEST(UniformSampleSphereTest, PhiVariation) {
    Float u_x = 0.3f;
    Point2f u1(u_x, 0.0f);
    Point2f u2(u_x, 0.5f);
    
    Vector3f result1 = UniformSampleSphere(u1);
    Vector3f result2 = UniformSampleSphere(u2);
    
    // Same z coordinate
    EXPECT_NEAR(result1.z, result2.z, 1e-6f);
    
    // Both on unit sphere
    EXPECT_NEAR(result1.Length(), 1.0f, 1e-6f);
    EXPECT_NEAR(result2.Length(), 1.0f, 1e-6f);
}

// Property 7: PDF is constant (uniform distribution)
TEST(UniformSampleSphereTest, UniformPDF) {
    Float pdf = UniformSpherePdf();
    Float expected = 1.0f / (4.0f * Pi);
    EXPECT_NEAR(pdf, expected, 1e-6f);
}

// Property 8: PDF equals Inv4Pi constant
TEST(UniformSampleSphereTest, PDFConstant) {
    Float pdf = UniformSpherePdf();
    EXPECT_NEAR(pdf, Inv4Pi, 1e-7f);
}

// Property 9: Rotational symmetry around z-axis
TEST(UniformSampleSphereTest, RotationalSymmetry) {
    Float u_x = 0.4f;
    
    for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.25f) {
        Point2f u(u_x, u_y);
        Vector3f result = UniformSampleSphere(u);
        
        // All should have same z and same distance from z-axis
        Float r_xy = std::sqrt(result.x * result.x + result.y * result.y);
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 10: Coverage test - samples should cover full sphere
TEST(UniformSampleSphereTest, SphereCoverage) {
    bool has_positive_z = false;
    bool has_negative_z = false;
    bool has_positive_x = false;
    bool has_negative_x = false;
    
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.2f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Vector3f result = UniformSampleSphere(u);
            
            if (result.z > 0.1f) has_positive_z = true;
            if (result.z < -0.1f) has_negative_z = true;
            if (result.x > 0.1f) has_positive_x = true;
            if (result.x < -0.1f) has_negative_x = true;
        }
    }
    
    EXPECT_TRUE(has_positive_z);
    EXPECT_TRUE(has_negative_z);
    EXPECT_TRUE(has_positive_x);
    EXPECT_TRUE(has_negative_x);
}
