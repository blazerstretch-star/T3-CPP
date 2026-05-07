#include <gtest/gtest.h>
#include "../src/nanort_functions.h"

using namespace nanort;

TEST(FindCutFromBinBufferTest, SimpleBinning) {
    BinBuffer<float> bins(4);
    
    // Setup bins with sample data
    for (unsigned int axis = 0; axis < 3; ++axis) {
        for (unsigned int i = 0; i < bins.bin_size; ++i) {
            Bin<float>& bin = bins.bin[axis * bins.bin_size + i];
            bin.count = i + 1;
            bin.bbox.bmin = real3<float>(0.0f, 0.0f, 0.0f);
            bin.bbox.bmax = real3<float>(1.0f, 1.0f, 1.0f);
        }
    }
    
    real3<float> bmin(0.0f, 0.0f, 0.0f);
    real3<float> bmax(4.0f, 4.0f, 4.0f);
    
    float cut_pos[3];
    int minCostAxis;
    
    bool result = FindCutFromBinBuffer(cut_pos, &minCostAxis, &bins, bmin, bmax);
    
    EXPECT_TRUE(result);
    EXPECT_GE(minCostAxis, 0);
    EXPECT_LE(minCostAxis, 2);
    
    // Check cut positions are within bounds
    for (int i = 0; i < 3; ++i) {
        EXPECT_GE(cut_pos[i], bmin[i]);
        EXPECT_LE(cut_pos[i], bmax[i]);
    }
}

TEST(FindCutFromBinBufferTest, UniformDistribution) {
    BinBuffer<float> bins(8);
    
    // Setup uniform distribution
    for (unsigned int axis = 0; axis < 3; ++axis) {
        for (unsigned int i = 0; i < bins.bin_size; ++i) {
            Bin<float>& bin = bins.bin[axis * bins.bin_size + i];
            bin.count = 10;
            bin.bbox.bmin = real3<float>(float(i), float(i), float(i));
            bin.bbox.bmax = real3<float>(float(i+1), float(i+1), float(i+1));
        }
    }
    
    real3<float> bmin(0.0f, 0.0f, 0.0f);
    real3<float> bmax(8.0f, 8.0f, 8.0f);
    
    float cut_pos[3];
    int minCostAxis;
    
    bool result = FindCutFromBinBuffer(cut_pos, &minCostAxis, &bins, bmin, bmax);
    
    EXPECT_TRUE(result);
    
    // With uniform distribution, cut should be near middle
    for (int i = 0; i < 3; ++i) {
        EXPECT_GT(cut_pos[i], 2.0f);
        EXPECT_LT(cut_pos[i], 6.0f);
    }
}

TEST(FindCutFromBinBufferTest, SkewedDistribution) {
    BinBuffer<float> bins(4);
    
    // Setup skewed distribution (more primitives on left)
    for (unsigned int axis = 0; axis < 3; ++axis) {
        for (unsigned int i = 0; i < bins.bin_size; ++i) {
            Bin<float>& bin = bins.bin[axis * bins.bin_size + i];
            bin.count = (bins.bin_size - i) * 10;  // More on left
            bin.bbox.bmin = real3<float>(float(i), float(i), float(i));
            bin.bbox.bmax = real3<float>(float(i+1), float(i+1), float(i+1));
        }
    }
    
    real3<float> bmin(0.0f, 0.0f, 0.0f);
    real3<float> bmax(4.0f, 4.0f, 4.0f);
    
    float cut_pos[3];
    int minCostAxis;
    
    bool result = FindCutFromBinBuffer(cut_pos, &minCostAxis, &bins, bmin, bmax);
    
    EXPECT_TRUE(result);
    EXPECT_GE(minCostAxis, 0);
    EXPECT_LE(minCostAxis, 2);
}

TEST(FindCutFromBinBufferTest, EmptyBins) {
    BinBuffer<float> bins(4);
    
    // All bins empty
    bins.clear();
    
    real3<float> bmin(0.0f, 0.0f, 0.0f);
    real3<float> bmax(4.0f, 4.0f, 4.0f);
    
    float cut_pos[3];
    int minCostAxis;
    
    bool result = FindCutFromBinBuffer(cut_pos, &minCostAxis, &bins, bmin, bmax);
    
    EXPECT_TRUE(result);
    EXPECT_GE(minCostAxis, 0);
    EXPECT_LE(minCostAxis, 2);
}
