#include <gtest/gtest.h>
#include <numeric>
#include "portfolio/MarkowitzOptimizer.hpp"

using portfolio::Matrix;
using portfolio::MarkowitzOptimizer;
using portfolio::PortfolioMetrics;

namespace {

PortfolioMetrics makeThreeAssetMetrics() {
    std::vector<double> mu = {0.08, 0.12, 0.15};
    Matrix cov({
        {0.010, 0.002, 0.001},
        {0.002, 0.015, 0.003},
        {0.001, 0.003, 0.020}
    });
    return PortfolioMetrics(mu, cov, 0.02);
}

double sumWeights(const std::vector<double>& w) {
    return std::accumulate(w.begin(), w.end(), 0.0);
}

} // namespace

TEST(MarkowitzOptimizerTest, MinVarianceWeightsSumToOne) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto result = opt.minimumVariancePortfolio();
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

TEST(MarkowitzOptimizerTest, MaxSharpeWeightsSumToOne) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto result = opt.maximumSharpePortfolio();
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

TEST(MarkowitzOptimizerTest, MinVarianceHasLowestVolatilityAmongFrontier) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto minVar = opt.minimumVariancePortfolio();

    // Compare against a few efficient portfolios at other target returns;
    // GMVP should have volatility <= any other efficient portfolio.
    for (double target : {0.09, 0.10, 0.13, 0.14}) {
        auto p = opt.efficientPortfolioForReturn(target);
        EXPECT_LE(minVar.volatility, p.volatility + 1e-9);
    }
}

TEST(MarkowitzOptimizerTest, EfficientPortfolioMatchesTargetReturn) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    double target = 0.11;
    auto result = opt.efficientPortfolioForReturn(target);
    EXPECT_NEAR(result.expectedReturn, target, 1e-6);
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

TEST(MarkowitzOptimizerTest, MaxSharpeHasHighestSharpeAmongCandidates) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto maxSharpe = opt.maximumSharpePortfolio();

    for (double target : {0.09, 0.10, 0.12, 0.13, 0.14}) {
        auto p = opt.efficientPortfolioForReturn(target);
        EXPECT_GE(maxSharpe.sharpeRatio + 1e-6, p.sharpeRatio);
    }
}

TEST(MarkowitzOptimizerTest, SingleAssetPortfolioIsFullyAllocated) {
    std::vector<double> mu = {0.10};
    Matrix cov({{0.05}});
    PortfolioMetrics metrics(mu, cov, 0.01);
    MarkowitzOptimizer opt(metrics);
    auto result = opt.minimumVariancePortfolio();
    ASSERT_EQ(result.weights.size(), 1u);
    EXPECT_NEAR(result.weights[0], 1.0, 1e-9);
}

// --- Long-only constraint verification (no shorting, no leverage) ---------

TEST(MarkowitzOptimizerTest, MinVarianceWeightsAreLongOnly) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto result = opt.minimumVariancePortfolio();
    for (double w : result.weights) {
        EXPECT_GE(w, -1e-9);
        EXPECT_LE(w, 1.0 + 1e-9);
    }
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

TEST(MarkowitzOptimizerTest, MaxSharpeWeightsAreLongOnly) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    auto result = opt.maximumSharpePortfolio();
    for (double w : result.weights) {
        EXPECT_GE(w, -1e-9);
        EXPECT_LE(w, 1.0 + 1e-9);
    }
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

TEST(MarkowitzOptimizerTest, EfficientFrontierWeightsAreLongOnly) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    for (double target : {0.085, 0.09, 0.10, 0.12, 0.13, 0.145}) {
        auto result = opt.efficientPortfolioForReturn(target);
        for (double w : result.weights) {
            EXPECT_GE(w, -1e-9);
            EXPECT_LE(w, 1.0 + 1e-9);
        }
        EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
    }
}

TEST(MarkowitzOptimizerTest, TargetReturnAboveMaxAssetReturnThrows) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    // Highest individual asset return is 0.15; long-only + fully-invested
    // portfolios cannot exceed the best single asset's return.
    EXPECT_THROW(opt.efficientPortfolioForReturn(0.20), std::runtime_error);
}

TEST(MarkowitzOptimizerTest, TargetReturnBelowMinAssetReturnThrows) {
    auto metrics = makeThreeAssetMetrics();
    MarkowitzOptimizer opt(metrics);
    // Lowest individual asset return is 0.08; long-only + fully-invested
    // portfolios cannot go below the worst single asset's return.
    EXPECT_THROW(opt.efficientPortfolioForReturn(0.01), std::runtime_error);
}

TEST(MarkowitzOptimizerTest, DoesNotDegenerateToUnconstrainedNegativeWeights) {
    // Construct a case where the unconstrained (closed-form) tangency
    // portfolio would short the low-return, high-variance asset. The
    // long-only optimizer must never produce a negative weight here.
    std::vector<double> mu = {0.05, 0.25, 0.10};
    Matrix cov({
        {0.05, 0.01, 0.00},
        {0.01, 0.20, 0.02},
        {0.00, 0.02, 0.03}
    });
    PortfolioMetrics metrics(mu, cov, 0.02);
    MarkowitzOptimizer opt(metrics);
    auto result = opt.maximumSharpePortfolio();
    for (double w : result.weights) {
        EXPECT_GE(w, -1e-9);
    }
    EXPECT_NEAR(sumWeights(result.weights), 1.0, 1e-9);
}

