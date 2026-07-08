#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <algorithm>
#include "portfolio/EfficientFrontier.hpp"

using portfolio::EfficientFrontier;
using portfolio::Matrix;
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

} // namespace

TEST(EfficientFrontierTest, GeneratesRequestedNumberOfPoints) {
    auto metrics = makeThreeAssetMetrics();
    EfficientFrontier frontier(metrics, 20);
    EXPECT_LE(frontier.points().size(), 20u);
    EXPECT_GT(frontier.points().size(), 0u);
}

TEST(EfficientFrontierTest, PointsAreSortedByReturn) {
    auto metrics = makeThreeAssetMetrics();
    EfficientFrontier frontier(metrics, 30);
    const auto& pts = frontier.points();
    EXPECT_TRUE(std::is_sorted(pts.begin(), pts.end(),
        [](const auto& a, const auto& b) { return a.expectedReturn < b.expectedReturn; }));
}

TEST(EfficientFrontierTest, AllVolatilitiesAreNonNegative) {
    auto metrics = makeThreeAssetMetrics();
    EfficientFrontier frontier(metrics, 15);
    for (const auto& p : frontier.points()) {
        EXPECT_GE(p.volatility, 0.0);
    }
}

TEST(EfficientFrontierTest, TooFewPointsThrows) {
    auto metrics = makeThreeAssetMetrics();
    EXPECT_THROW(EfficientFrontier(metrics, 1), std::invalid_argument);
}

TEST(EfficientFrontierTest, ExportToCsvProducesValidFile) {
    auto metrics = makeThreeAssetMetrics();
    EfficientFrontier frontier(metrics, 10);
    std::string path = "test_frontier_export.csv";
    frontier.exportToCsv(path);

    std::ifstream in(path);
    ASSERT_TRUE(in.is_open());
    std::string header;
    std::getline(in, header);
    EXPECT_EQ(header, "ExpectedReturn,Volatility,SharpeRatio");

    std::size_t lineCount = 0;
    std::string line;
    while (std::getline(in, line)) ++lineCount;
    EXPECT_EQ(lineCount, frontier.points().size());

    in.close();
    std::remove(path.c_str());
}
