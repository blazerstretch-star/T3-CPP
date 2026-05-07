#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

TEST(UniformHemispherePdfTest, BasicValue) {
    Float pdf = UniformHemispherePdf();
    Float expected = 1.0f / (2.0f * Pi);
    EXPECT_NEAR(pdf, expected, 1e-6);
}

TEST(UniformHemispherePdfTest, MatchesConstant) {
    Float pdf = UniformHemispherePdf();
    EXPECT_FLOAT_EQ(pdf, Inv2Pi);
}

TEST(UniformHemispherePdfTest, MathematicalRelationship) {
    Float pdf = UniformHemispherePdf();
    Float product = 2.0f * Pi * pdf;
    EXPECT_NEAR(product, 1.0f, 1e-5);
}

TEST(UniformHemispherePdfTest, PositiveValue) {
    Float pdf = UniformHemispherePdf();
    EXPECT_GT(pdf, 0.0f);
}

TEST(UniformHemispherePdfTest, LessThanOne) {
    Float pdf = UniformHemispherePdf();
    EXPECT_LT(pdf, 1.0f);
}

TEST(UniformHemispherePdfTest, TwiceSpherePdf) {
    Float hemispherePdf = UniformHemispherePdf();
    Float spherePdf = UniformSpherePdf();
    EXPECT_NEAR(hemispherePdf, 2.0f * spherePdf, 1e-6);
}

TEST(UniformHemispherePdfTest, ConstantFunction) {
    Float pdf1 = UniformHemispherePdf();
    Float pdf2 = UniformHemispherePdf();
    Float pdf3 = UniformHemispherePdf();
    EXPECT_FLOAT_EQ(pdf1, pdf2);
    EXPECT_FLOAT_EQ(pdf1, pdf3);
}

TEST(UniformHemispherePdfTest, ReasonableRange) {
    Float pdf = UniformHemispherePdf();
    EXPECT_GT(pdf, 0.15f);
    EXPECT_LT(pdf, 0.16f);
}

TEST(UniformHemispherePdfTest, RelationshipWithPi) {
    Float pdf = UniformHemispherePdf();
    EXPECT_NEAR(pdf * Pi, 0.5f, 1e-5);
}
