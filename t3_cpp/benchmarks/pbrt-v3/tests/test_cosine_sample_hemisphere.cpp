#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: All samples lie on unit hemisphere
TEST(CosineSampleHemisphereTest, UnitLength) {
    Point2f samples[] = {
        Point2f(0.0f, 0.0f), Point2f(0.5f, 0.5f), Point2f(1.0f, 1.0f),
        Point2f(0.25f, 0.75f), Point2f(0.7f, 0.3f)
    };
    
    for (const auto& u : samples) {
        Vector3f result = CosineSampleHemisphere(u);
        EXPECT_NEAR(result.Length(), 1.0f, 1e-6f);
    }
}

// Property 2: Z-coordinate is always non-negative
TEST(CosineSampleHemisphereTest, PositiveZ) {
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.1f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Vector3f result = CosineSampleHemisphere(u);
            EXPECT_GE(result.z, -1e-6f);
        }
    }
}

// Property 3: Center sample points upward
TEST(CosineSampleHemisphereTest, CenterSample) {
    Point2f u(0.5f, 0.5f);
    Vector3f result = CosineSampleHemisphere(u);
    
    // Center of disk projects to top of hemisphere
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
    EXPECT_NEAR(result.z, 1.0f, 1e-6f);
}

// Property 4: PDF is proportional to cosine
TEST(CosineSampleHemisphereTest, PDFFormula) {
    Float cosTheta_values[] = {0.0f, 0.25f, 0.5f, 0.707f, 1.0f};
    
    for (Float cosTheta : cosTheta_values) {
        Float pdf = CosineHemispherePdf(cosTheta);
        Float expected = cosTheta * InvPi;
        EXPECT_NEAR(pdf, expected, 1e-6f);
    }
}

// Property 5: PDF at normal direction (cosTheta = 1)
TEST(CosineSampleHemisphereTest, PDFAtNormal) {
    Float pdf = CosineHemispherePdf(1.0f);
    EXPECT_NEAR(pdf, InvPi, 1e-7f);
}

// Property 6: PDF at grazing angle (cosTheta = 0)
TEST(CosineSampleHemisphereTest, PDFAtGrazing) {
    Float pdf = CosineHemispherePdf(0.0f);
    EXPECT_NEAR(pdf, 0.0f, 1e-7f);
}

// Property 7: PDF is monotonically increasing with cosTheta
TEST(CosineSampleHemisphereTest, PDFMonotonicity) {
    Float prev_pdf = 0.0f;
    
    for (Float cosTheta = 0.0f; cosTheta <= 1.0f; cosTheta += 0.1f) {
        Float curr_pdf = CosineHemispherePdf(cosTheta);
        EXPECT_GE(curr_pdf, prev_pdf - 1e-6f);
        prev_pdf = curr_pdf;
    }
}

// Property 8: Samples cover hemisphere
TEST(CosineSampleHemisphereTest, HemisphereCoverage) {
    bool has_high_z = false;
    bool has_low_z = false;
    bool has_positive_x = false;
    bool has_negative_x = false;
    
    for (Float u_x = 0.0f; u_x <= 1.0f; u_x += 0.2f) {
        for (Float u_y = 0.0f; u_y <= 1.0f; u_y += 0.2f) {
            Point2f u(u_x, u_y);
            Vector3f result = CosineSampleHemisphere(u);
            
            if (result.z > 0.8f) has_high_z = true;
            if (result.z < 0.3f) has_low_z = true;
            if (result.x > 0.1f) has_positive_x = true;
            if (result.x < -0.1f) has_negative_x = true;
        }
    }
    
    EXPECT_TRUE(has_high_z);
    EXPECT_TRUE(has_low_z);
    EXPECT_TRUE(has_positive_x);
    EXPECT_TRUE(has_negative_x);
}

// Property 9: PDF linear relationship
TEST(CosineSampleHemisphereTest, PDFLinearRelationship) {
    Float cosA = 0.3f;
    Float cosB = 0.6f;
    
    Float pdfA = CosineHemispherePdf(cosA);
    Float pdfB = CosineHemispherePdf(cosB);
    
    // PDF ratio should equal cosine ratio
    EXPECT_NEAR(pdfB / pdfA, cosB / cosA, 1e-5f);
}

// Property 10: Consistency between sampling and PDF
TEST(CosineSampleHemisphereTest, SamplingPDFConsistency) {
    Point2f u(0.7f, 0.3f);
    Vector3f sample = CosineSampleHemisphere(u);
    
    // Sample should be on unit sphere
    EXPECT_NEAR(sample.Length(), 1.0f, 1e-6f);
    
    // PDF at this sample
    Float pdf = CosineHemispherePdf(sample.z);
    
    // PDF should be positive and reasonable
    EXPECT_GT(pdf, 0.0f);
    EXPECT_LT(pdf, 1.0f);  // PDF should be less than 1/π ≈ 0.318
}
