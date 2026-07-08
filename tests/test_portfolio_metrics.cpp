#include <gtest/gtest.h>
#include <cmath>
#include "portfolio/PortfolioMetrics.hpp"

using portfolio::Matrix;
using portfolio::PortfolioMetrics;

namespace {

PortfolioMetrics makeTwoAssetMetrics() {
    std::vector<double> mu = {0.10, 0.20};
    // Diagonal covariance: no correlation, var(A)=0.04, var(B)=0.09
    Matrix cov({{0.04, 0.0}, {0.0, 0.09}});
    return PortfolioMetrics(mu, cov, 0.02);
}

} // namespace

TEST(PortfolioMetricsTest, ReturnIsWeightedAverage) {
    auto metrics = makeTwoAssetMetrics();
    std::vector<double> w = {0.5, 0.5};
    EXPECT_NEAR(metrics.portfolioReturn(w), 0.15, 1e-9);
}

TEST(PortfolioMetricsTest, VolatilityUncorrelatedAssets) {
    auto metrics = makeTwoAssetMetrics();
    std::vector<double> w = {0.5, 0.5};
    // var = 0.25*0.04 + 0.25*0.09 = 0.0325 -> vol = sqrt(0.0325)
    double expectedVol = std::sqrt(0.25 * 0.04 + 0.25 * 0.09);
    EXPECT_NEAR(metrics.portfolioVolatility(w), expectedVol, 1e-9);
}

TEST(PortfolioMetricsTest, SharpeRatioComputation) {
    auto metrics = makeTwoAssetMetrics();
    std::vector<double> w = {1.0, 0.0};
    // Only asset A: return=0.10, vol=0.2, rf=0.02 -> sharpe = 0.08/0.2 = 0.4
    EXPECT_NEAR(metrics.sharpeRatio(w), 0.4, 1e-9);
}

TEST(PortfolioMetricsTest, EvaluateReturnsAllMetrics) {
    auto metrics = makeTwoAssetMetrics();
    std::vector<double> w = {0.5, 0.5};
    auto result = metrics.evaluate(w);
    EXPECT_NEAR(result.expectedReturn, 0.15, 1e-9);
    EXPECT_NEAR(result.volatility, metrics.portfolioVolatility(w), 1e-9);
    EXPECT_NEAR(result.sharpeRatio, metrics.sharpeRatio(w), 1e-9);
    EXPECT_EQ(result.weights, w);
}

TEST(PortfolioMetricsTest, MismatchedWeightsThrows) {
    auto metrics = makeTwoAssetMetrics();
    std::vector<double> w = {1.0};
    EXPECT_THROW(metrics.portfolioReturn(w), std::invalid_argument);
}

TEST(PortfolioMetricsTest, MismatchedCovarianceDimensionsThrows) {
    std::vector<double> mu = {0.1, 0.2, 0.3};
    Matrix cov({{0.04, 0.0}, {0.0, 0.09}}); // 2x2, but mu has 3 assets
    EXPECT_THROW(PortfolioMetrics(mu, cov), std::invalid_argument);
}

TEST(PortfolioMetricsTest, ZeroVolatilityGivesZeroSharpe) {
    std::vector<double> mu = {0.05};
    Matrix cov({{0.0}});
    PortfolioMetrics metrics(mu, cov, 0.01);
    EXPECT_DOUBLE_EQ(metrics.sharpeRatio({1.0}), 0.0);
}
