#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(VectorMath, Vdot) {
    nanort::real3<float> a(1.0f, 0.0f, 0.0f);
    nanort::real3<float> b(0.0f, 1.0f, 0.0f);
    EXPECT_FLOAT_EQ(nanort::vdot(a, b), 0.0f);
    
    nanort::real3<float> c(1.0f, 2.0f, 3.0f);
    nanort::real3<float> d(4.0f, 5.0f, 6.0f);
    EXPECT_FLOAT_EQ(nanort::vdot(c, d), 32.0f);
}
