#include <gtest/gtest.h>
#include "portfolio/ReturnsCalculator.hpp"

using portfolio::PriceTable;
using portfolio::ReturnsCalculator;

namespace {

PriceTable makeSimpleTable() {
    PriceTable t;
    t.tickers = {"A", "B"};
    t.dates = {"d1", "d2", "d3"};
    t.prices = {
        {100.0, 50.0},
        {110.0, 55.0}, // +10% both
        {99.0, 49.5}   // -10% both
    };
    return t;
}

} // namespace

TEST(ReturnsCalculatorTest, ComputesPeriodicReturns) {
    ReturnsCalculator calc(makeSimpleTable(), 252);
    const auto& returns = calc.periodicReturns();
    ASSERT_EQ(returns.size(), 2u);
    EXPECT_NEAR(returns[0][0], 0.10, 1e-9);
    EXPECT_NEAR(returns[0][1], 0.10, 1e-9);
    EXPECT_NEAR(returns[1][0], -0.10, 1e-9);
    EXPECT_NEAR(returns[1][1], -0.10, 1e-9);
}

TEST(ReturnsCalculatorTest, MeanPeriodicReturns) {
    ReturnsCalculator calc(makeSimpleTable(), 252);
    auto means = calc.meanPeriodicReturns();
    ASSERT_EQ(means.size(), 2u);
    EXPECT_NEAR(means[0], 0.0, 1e-9);
    EXPECT_NEAR(means[1], 0.0, 1e-9);
}

TEST(ReturnsCalculatorTest, AnnualizedReturnsScaling) {
    ReturnsCalculator calc(makeSimpleTable(), 252);
    auto meanReturns = calc.meanPeriodicReturns();
    auto annualized = calc.annualizedExpectedReturns();
    for (std::size_t i = 0; i < meanReturns.size(); ++i) {
        EXPECT_NEAR(annualized[i], meanReturns[i] * 252, 1e-9);
    }
}

TEST(ReturnsCalculatorTest, NumAssetsAndPeriods) {
    ReturnsCalculator calc(makeSimpleTable(), 252);
    EXPECT_EQ(calc.numAssets(), 2u);
    EXPECT_EQ(calc.numPeriods(), 2u);
    EXPECT_EQ(calc.periodsPerYear(), 252);
}

TEST(ReturnsCalculatorTest, TooFewRowsThrows) {
    PriceTable t;
    t.tickers = {"A"};
    t.dates = {"d1"};
    t.prices = {{100.0}};
    EXPECT_THROW(ReturnsCalculator(t, 252), std::invalid_argument);
}

TEST(ReturnsCalculatorTest, NonPositivePriceThrows) {
    PriceTable t;
    t.tickers = {"A"};
    t.dates = {"d1", "d2"};
    t.prices = {{0.0}, {10.0}};
    EXPECT_THROW(ReturnsCalculator(t, 252), std::invalid_argument);
}
