#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

using namespace nanort;

TEST(ScalarMultiplyTest, BasicMultiplication) {
    real3<float> v(1.0f, 2.0f, 3.0f);
    real3<float> result = 2.0f * v;
    
    EXPECT_FLOAT_EQ(result[0], 2.0f);
    EXPECT_FLOAT_EQ(result[1], 4.0f);
    EXPECT_FLOAT_EQ(result[2], 6.0f);
}

TEST(ScalarMultiplyTest, ZeroScalar) {
    real3<float> v(1.0f, 2.0f, 3.0f);
    real3<float> result = 0.0f * v;
    
    EXPECT_FLOAT_EQ(result[0], 0.0f);
    EXPECT_FLOAT_EQ(result[1], 0.0f);
    EXPECT_FLOAT_EQ(result[2], 0.0f);
}

TEST(ScalarMultiplyTest, NegativeScalar) {
    real3<float> v(1.0f, 2.0f, 3.0f);
    real3<float> result = -2.0f * v;
    
    EXPECT_FLOAT_EQ(result[0], -2.0f);
    EXPECT_FLOAT_EQ(result[1], -4.0f);
    EXPECT_FLOAT_EQ(result[2], -6.0f);
}

TEST(ScalarMultiplyTest, FractionalScalar) {
    real3<float> v(2.0f, 4.0f, 6.0f);
    real3<float> result = 0.5f * v;
    
    EXPECT_FLOAT_EQ(result[0], 1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_FLOAT_EQ(result[2], 3.0f);
}
