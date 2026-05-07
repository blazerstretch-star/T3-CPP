#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: All samples lie on unit hemisphere
TEST(UniformSampleHemisphereTest, UnitLength) {
    Point2f samples[] = {
        Point2f(0.0f, 0.0f), Point2f(0.5f, 0.5f), Point2f(1.0f, 1.0f),
        Point2f(0.25f, 0.75f), Point2f(0.7f, 0.3f)
    };
    
    for (const auto& u : samples) {
        Vector3f result = UniformSampleHemisphere(u);
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 2: Z-coordinate is always non-negative (hemisphere constraint)
TEST(UniformSampleHemisphereTest, PositiveZ) {
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.1f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Vector3f result = UniformSampleHemisphere(u);
            EXPECT_GE(result.z, -1e-6f);
        }
    }
}

// Property 3: Base of hemisphere (u.x = 0)
TEST(UniformSampleHemisphereTest, BaseOfHemisphere) {
    Point2f u(0.0f, 0.5f);
    Vector3f result = UniformSampleHemisphere(u);
    EXPECT_NEAR(result.z, 0.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 4: Top of hemisphere (u.x = 1)
TEST(UniformSampleHemisphereTest, TopOfHemisphere) {
    Point2f u(1.0f, 0.5f);
    Vector3f result = UniformSampleHemisphere(u);
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
    EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
}

// Property 5: Z-coordinate equals u.x
TEST(UniformSampleHemisphereTest, ZEqualsUX) {
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.1f) {
        Point2f u(u_x, 0.5f);
        Vector3f result = UniformSampleHemisphere(u);
        EXPECT_NEAR(result.z, u_x, 1e-6f);
    }
}

// Property 6: PDF is constant (uniform distribution)
TEST(UniformSampleHemisphereTest, UniformPDF) {
    Float pdf = UniformHemispherePdf();
    Float expected = 1.0f / (2.0f * Pi);
    EXPECT_NEAR(pdf, expected, 1e-6f);
}

// Property 7: PDF equals Inv2Pi constant
TEST(UniformSampleHemisphereTest, PDFConstant) {
    Float pdf = UniformHemispherePdf();
    EXPECT_NEAR(pdf, Inv2Pi, 1e-7f);
}

// Property 8: Rotational symmetry around z-axis
TEST(UniformSampleHemisphereTest, RotationalSymmetry) {
    Float u_x = 0.6f;
    
    for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.25f) {
        Point2f u(u_x, u_y);
        Vector3f result = UniformSampleHemisphere(u);
        
        // All should have same z
        EXPECT_NEAR(result.z, u_x, 1e-6f);
        
        // All on unit sphere
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 9: Hemisphere coverage
TEST(UniformSampleHemisphereTest, HemisphereCoverage) {
    bool has_near_zero_z = false;
    bool has_near_one_z = false;
    bool has_positive_x = false;
    bool has_negative_x = false;
    
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.2f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Vector3f result = UniformSampleHemisphere(u);
            
            if (result.z < 0.1f) has_near_zero_z = true;
            if (result.z > 0.9f) has_near_one_z = true;
            if (result.x > 0.1f) has_positive_x = true;
            if (result.x < -0.1f) has_negative_x = true;
        }
    }
    
    EXPECT_TRUE(has_near_zero_z);
    EXPECT_TRUE(has_near_one_z);
    EXPECT_TRUE(has_positive_x);
    EXPECT_TRUE(has_negative_x);
}

// Property 10: Relationship between x,y,z components
TEST(UniformSampleHemisphereTest, ComponentRelationship) {
    Point2f u(0.5f, 0.0f);
    Vector3f result = UniformSampleHemisphere(u);
    
    // x^2 + y^2 + z^2 = 1
    Float sum_squares = result.x * result.x + result.y * result.y + result.z * result.z;
    EXPECT_NEAR(sum_squares, 1.0f, 1e-6f);
    
    // z = u.x
    EXPECT_NEAR(result.z, 0.5f, 1e-6f);
    
    // x^2 + y^2 = 1 - z^2
    Float xy_sum = result.x * result.x + result.y * result.y;
    Float expected_xy = 1.0f - result.z * result.z;
    EXPECT_NEAR(xy_sum, expected_xy, 1e-6f);
}
