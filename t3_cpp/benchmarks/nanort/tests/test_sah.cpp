#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(BVH, SAH) {
    float sah = nanort::SAH<float>(10, 100.0f, 20, 200.0f, 0.01f, 0.2f, 1.0f);
    
    float expected = 2.0f * 0.2f + (100.0f * 0.01f) * 10.0f * 1.0f + (200.0f * 0.01f) * 20.0f * 1.0f;
    EXPECT_FLOAT_EQ(sah, expected);
}
