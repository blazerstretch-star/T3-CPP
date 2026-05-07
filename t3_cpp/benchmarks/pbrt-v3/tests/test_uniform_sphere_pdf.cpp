#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

TEST(UniformSpherePdfTest, BasicValue) {
    Float pdf = UniformSpherePdf();
    Float expected = 1.0f / (4.0f * Pi);
    EXPECT_NEAR(pdf, expected, 1e-6);
}

TEST(UniformSpherePdfTest, MatchesConstant) {
    Float pdf = UniformSpherePdf();
    EXPECT_FLOAT_EQ(pdf, Inv4Pi);
}

TEST(UniformSpherePdfTest, MathematicalRelationship) {
    Float pdf = UniformSpherePdf();
    Float product = 4.0f * Pi * pdf;
    EXPECT_NEAR(product, 1.0f, 1e-5);
}

TEST(UniformSpherePdfTest, PositiveValue) {
    Float pdf = UniformSpherePdf();
    EXPECT_GT(pdf, 0.0f);
}

TEST(UniformSpherePdfTest, LessThanOne) {
    Float pdf = UniformSpherePdf();
    EXPECT_LT(pdf, 1.0f);
}

TEST(UniformSpherePdfTest, ConstantFunction) {
    Float pdf1 = UniformSpherePdf();
    Float pdf2 = UniformSpherePdf();
    Float pdf3 = UniformSpherePdf();
    EXPECT_FLOAT_EQ(pdf1, pdf2);
    EXPECT_FLOAT_EQ(pdf1, pdf3);
}

TEST(UniformSpherePdfTest, ReasonableRange) {
    Float pdf = UniformSpherePdf();
    EXPECT_GT(pdf, 0.07f);
    EXPECT_LT(pdf, 0.08f);
}
