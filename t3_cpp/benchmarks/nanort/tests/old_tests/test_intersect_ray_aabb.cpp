#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

TEST(Intersection, IntersectRayAABB) {
    float bmin[3] = {-1.0f, -1.0f, -1.0f};
    float bmax[3] = {1.0f, 1.0f, 1.0f};
    
    nanort::real3<float> ray_org(0.0f, 0.0f, -5.0f);
    nanort::real3<float> ray_dir(0.0f, 0.0f, 1.0f);
    nanort::real3<float> ray_inv_dir = nanort::vsafe_inverse(ray_dir);
    
    int ray_dir_sign[3] = {0, 0, 0};
    float tmin, tmax;
    
    bool hit = nanort::IntersectRayAABB(&tmin, &tmax, 0.0f, 1000.0f,
                                       bmin, bmax, ray_org, ray_inv_dir, ray_dir_sign);
    
    EXPECT_TRUE(hit);
    EXPECT_FLOAT_EQ(tmin, 4.0f);
    EXPECT_FLOAT_EQ(tmax, 6.0f * 1.00000024f);
}
