#include <gtest/gtest.h>
#include "../src/nanort_functions.h"
#include <cmath>

TEST(VectorMath, Vlength) {
    nanort::real3<float> a(3.0f, 4.0f, 0.0f);
    EXPECT_FLOAT_EQ(nanort::vlength(a), 5.0f);
    
    nanort::real3<float> b(1.0f, 1.0f, 1.0f);
    EXPECT_NEAR(nanort::vlength(b), std::sqrt(3.0f), 1e-6f);
}
