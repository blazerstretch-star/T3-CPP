#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(VectorMath, Vnormalize) {
    nanort::real3<float> a(3.0f, 4.0f, 0.0f);
    nanort::real3<float> n = nanort::vnormalize(a);
    
    EXPECT_NEAR(n[0], 0.6f, 1e-6f);
    EXPECT_NEAR(n[1], 0.8f, 1e-6f);
    EXPECT_NEAR(n[2], 0.0f, 1e-6f);
    EXPECT_NEAR(nanort::vlength(n), 1.0f, 1e-6f);
}
