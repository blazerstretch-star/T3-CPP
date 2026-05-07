#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

TEST(VectorMath, VsafeInverse) {
    nanort::real3<float> a(2.0f, 4.0f, 8.0f);
    nanort::real3<float> inv = nanort::vsafe_inverse(a);
    
    EXPECT_FLOAT_EQ(inv[0], 0.5f);
    EXPECT_FLOAT_EQ(inv[1], 0.25f);
    EXPECT_FLOAT_EQ(inv[2], 0.125f);
    
    nanort::real3<float> b(0.0f, 1.0f, -0.0f);
    nanort::real3<float> inv2 = nanort::vsafe_inverse(b);
    EXPECT_TRUE(std::isinf(inv2[0]));
    EXPECT_FLOAT_EQ(inv2[1], 1.0f);
}
