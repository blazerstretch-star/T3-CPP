#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: PDF is always non-negative
TEST(CosineHemispherePdfTest, NonNegative) {
    for (Float cosTheta = 0.0f; cosTheta <= 1.0f; cosTheta += 0.1f) {
        Float pdf = CosineHemispherePdf(cosTheta);
        EXPECT_GE(pdf, 0.0f);
    }
}

// Property 2: PDF at normal direction (cosTheta = 1)
TEST(CosineHemispherePdfTest, NormalDirection) {
    Float pdf = CosineHemispherePdf(1.0f);
    EXPECT_NEAR(pdf, InvPi, 1e-7f);
}

// Property 3: PDF at grazing angle (cosTheta = 0)
TEST(CosineHemispherePdfTest, GrazingAngle) {
    Float pdf = CosineHemispherePdf(0.0f);
    EXPECT_NEAR(pdf, 0.0f, 1e-7f);
}

// Property 4: Linear relationship - PDF = cosTheta / π
TEST(CosineHemispherePdfTest, LinearFormula) {
    Float cosTheta_values[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    
    for (Float cosTheta : cosTheta_values) {
        Float pdf = CosineHemispherePdf(cosTheta);
        Float expected = cosTheta * InvPi;
        EXPECT_NEAR(pdf, expected, 1e-6f);
    }
}

// Property 5: PDF at 45 degrees
TEST(CosineHemispherePdfTest, FortyFiveDegrees) {
    Float cosTheta45 = std::sqrt(2.0f) / 2.0f;
    Float pdf = CosineHemispherePdf(cosTheta45);
    Float expected = cosTheta45 * InvPi;
    EXPECT_NEAR(pdf, expected, 1e-6f);
}

// Property 6: PDF at 60 degrees
TEST(CosineHemispherePdfTest, SixtyDegrees) {
    Float cosTheta60 = 0.5f;
    Float pdf = CosineHemispherePdf(cosTheta60);
    Float expected = 0.5f * InvPi;
    EXPECT_NEAR(pdf, expected, 1e-6f);
}

// Property 7: Monotonicity - PDF increases with cosTheta
TEST(CosineHemispherePdfTest, Monotonicity) {
    Float prev_pdf = 0.0f;
    
    for (Float cosTheta = 0.0f; cosTheta <= 1.0f; cosTheta += 0.05f) {
        Float curr_pdf = CosineHemispherePdf(cosTheta);
        EXPECT_GE(curr_pdf, prev_pdf - 1e-6f);
        prev_pdf = curr_pdf;
    }
}

// Property 8: Proportionality - doubling cosTheta doubles PDF
TEST(CosineHemispherePdfTest, Proportionality) {
    Float cosA = 0.3f;
    Float cosB = 0.6f;
    
    Float pdfA = CosineHemispherePdf(cosA);
    Float pdfB = CosineHemispherePdf(cosB);
    
    EXPECT_NEAR(pdfB / pdfA, cosB / cosA, 1e-5f);
}

// Property 9: Maximum PDF is at normal
TEST(CosineHemispherePdfTest, MaximumAtNormal) {
    Float pdf_normal = CosineHemispherePdf(1.0f);
    
    for (Float cosTheta = 0.0f; cosTheta < 1.0f; cosTheta += 0.1f) {
        Float pdf = CosineHemispherePdf(cosTheta);
        EXPECT_LE(pdf, pdf_normal + 1e-6f);
    }
}

// Property 10: Specific value verification
TEST(CosineHemispherePdfTest, SpecificValues) {
    // At cosTheta = 0.5, PDF = 0.5/π ≈ 0.159155
    Float pdf = CosineHemispherePdf(0.5f);
    EXPECT_NEAR(pdf, 0.5f / Pi, 1e-6f);
    
    // At cosTheta = 1.0, PDF = 1/π ≈ 0.31831
    Float pdf_max = CosineHemispherePdf(1.0f);
    EXPECT_NEAR(pdf_max, 1.0f / Pi, 1e-6f);
}
