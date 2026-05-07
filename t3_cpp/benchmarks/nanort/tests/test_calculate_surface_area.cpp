#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(BoundingBox, CalculateSurfaceArea) {
    nanort::real3<float> min(0.0f, 0.0f, 0.0f);
    nanort::real3<float> max(1.0f, 1.0f, 1.0f);
    
    float area = nanort::CalculateSurfaceArea(min, max);
    EXPECT_FLOAT_EQ(area, 6.0f);
    
    nanort::real3<float> min2(0.0f, 0.0f, 0.0f);
    nanort::real3<float> max2(2.0f, 3.0f, 4.0f);
    float area2 = nanort::CalculateSurfaceArea(min2, max2);
    EXPECT_FLOAT_EQ(area2, 2.0f * (2.0f*3.0f + 3.0f*4.0f + 4.0f*2.0f));
}
