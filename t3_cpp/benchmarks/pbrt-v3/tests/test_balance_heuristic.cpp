#include <gtest/gtest.h>
#include "pbrt_functions.h"

using namespace pbrt;

// Property 1: Result is always in [0, 1]
TEST(BalanceHeuristicTest, BoundedRange) {
    Float result1 = BalanceHeuristic(1, 0.5f, 1, 0.5f);
    Float result2 = BalanceHeuristic(2, 0.8f, 1, 0.2f);
    Float result3 = BalanceHeuristic(1, 0.1f, 3, 0.9f);
    
    EXPECT_GE(result1, 0.0f);
    EXPECT_LE(result1, 1.0f);
    EXPECT_GE(result2, 0.0f);
    EXPECT_LE(result2, 1.0f);
    EXPECT_GE(result3, 0.0f);
    EXPECT_LE(result3, 1.0f);
}

// Property 2: Equal weights produce 0.5
TEST(BalanceHeuristicTest, EqualWeights) {
    Float result = BalanceHeuristic(1, 0.5f, 1, 0.5f);
    EXPECT_NEAR(result, 0.5f, 1e-6f);
    
    Float result2 = BalanceHeuristic(2, 0.3f, 2, 0.3f);
    EXPECT_NEAR(result2, 0.5f, 1e-6f);
}

// Property 3: Complementary weights sum to 1
TEST(BalanceHeuristicTest, ComplementarySumToOne) {
    int nf = 2, ng = 3;
    Float fPdf = 0.6f, gPdf = 0.4f;
    
    Float wf = BalanceHeuristic(nf, fPdf, ng, gPdf);
    Float wg = BalanceHeuristic(ng, gPdf, nf, fPdf);
    
    EXPECT_NEAR(wf + wg, 1.0f, 1e-6f);
}

// Property 4: Dominant strategy gets higher weight
TEST(BalanceHeuristicTest, DominantStrategy) {
    Float result = BalanceHeuristic(2, 0.8f, 1, 0.2f);
    EXPECT_GT(result, 0.5f);
    
    Float result2 = BalanceHeuristic(1, 0.2f, 2, 0.8f);
    EXPECT_LT(result2, 0.5f);
}

// Property 5: Zero PDF in second strategy gives weight 1
TEST(BalanceHeuristicTest, ZeroSecondPDF) {
    Float result = BalanceHeuristic(1, 0.5f, 1, 0.0f);
    EXPECT_NEAR(result, 1.0f, 1e-6f);
}

// Property 6: Zero PDF in first strategy gives weight 0
TEST(BalanceHeuristicTest, ZeroFirstPDF) {
    Float result = BalanceHeuristic(1, 0.0f, 1, 0.5f);
    EXPECT_NEAR(result, 0.0f, 1e-6f);
}

// Property 7: Scaling both PDFs doesn't change result
TEST(BalanceHeuristicTest, ScaleInvariance) {
    Float result1 = BalanceHeuristic(2, 0.3f, 3, 0.4f);
    Float result2 = BalanceHeuristic(2, 0.6f, 3, 0.8f);  // 2x scaled
    
    EXPECT_NEAR(result1, result2, 1e-6f);
}

// Property 8: Increasing sample count increases weight
TEST(BalanceHeuristicTest, SampleCountEffect) {
    Float pdf1 = 0.5f, pdf2 = 0.5f;
    
    Float w1 = BalanceHeuristic(1, pdf1, 1, pdf2);
    Float w2 = BalanceHeuristic(2, pdf1, 1, pdf2);
    Float w3 = BalanceHeuristic(3, pdf1, 1, pdf2);
    
    EXPECT_GT(w2, w1);
    EXPECT_GT(w3, w2);
}

// Property 9: Known result verification
TEST(BalanceHeuristicTest, KnownResult) {
    // nf=1, fPdf=0.6, ng=2, gPdf=0.3
    // Result = (1*0.6) / (1*0.6 + 2*0.3) = 0.6 / 1.2 = 0.5
    Float result = BalanceHeuristic(1, 0.6f, 2, 0.3f);
    EXPECT_NEAR(result, 0.5f, 1e-6f);
}

// Property 10: Monotonicity with respect to first PDF
TEST(BalanceHeuristicTest, MonotonicityFirstPDF) {
    int nf = 1, ng = 1;
    Float gPdf = 0.5f;
    
    Float prev_weight = 0.0f;
    for (Float fPdf = 0.1f; fPdf <= 0.9f; fPdf += 0.1f) {
        Float weight = BalanceHeuristic(nf, fPdf, ng, gPdf);
        EXPECT_GE(weight, prev_weight - 1e-6f);
        prev_weight = weight;
    }
}
