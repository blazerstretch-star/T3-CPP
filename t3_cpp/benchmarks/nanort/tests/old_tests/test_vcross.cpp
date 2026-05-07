#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(VectorMath, Vcross) {
    nanort::real3<float> a(1.0f, 0.0f, 0.0f);
    nanort::real3<float> b(0.0f, 1.0f, 0.0f);
    nanort::real3<float> c = nanort::vcross(a, b);
    
    EXPECT_FLOAT_EQ(c[0], 0.0f);
    EXPECT_FLOAT_EQ(c[1], 0.0f);
    EXPECT_FLOAT_EQ(c[2], 1.0f);
}
