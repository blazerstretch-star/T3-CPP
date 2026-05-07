#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

using namespace nanort;

TEST(VnegTest, BasicNegation) {
    real3<float> v(1.0f, 2.0f, 3.0f);
    real3<float> result = vneg(v);
    
    EXPECT_FLOAT_EQ(result[0], -1.0f);
    EXPECT_FLOAT_EQ(result[1], -2.0f);
    EXPECT_FLOAT_EQ(result[2], -3.0f);
}

TEST(VnegTest, NegativeValues) {
    real3<float> v(-1.0f, -2.0f, -3.0f);
    real3<float> result = vneg(v);
    
    EXPECT_FLOAT_EQ(result[0], 1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_FLOAT_EQ(result[2], 3.0f);
}

TEST(VnegTest, ZeroVector) {
    real3<float> v(0.0f, 0.0f, 0.0f);
    real3<float> result = vneg(v);
    
    EXPECT_FLOAT_EQ(result[0], 0.0f);
    EXPECT_FLOAT_EQ(result[1], 0.0f);
    EXPECT_FLOAT_EQ(result[2], 0.0f);
}

TEST(VnegTest, MixedValues) {
    real3<float> v(1.0f, -2.0f, 3.0f);
    real3<float> result = vneg(v);
    
    EXPECT_FLOAT_EQ(result[0], -1.0f);
    EXPECT_FLOAT_EQ(result[1], 2.0f);
    EXPECT_FLOAT_EQ(result[2], -3.0f);
}
