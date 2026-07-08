#include <gtest/gtest.h>
#include "portfolio/CovarianceMatrix.hpp"

using portfolio::CovarianceMatrix;
using portfolio::PriceTable;
using portfolio::ReturnsCalculator;

namespace {

// Two perfectly correlated assets: B is always exactly 2x A's price scale,
// so their returns are identical each period -> covariance == variance.
PriceTable makeCorrelatedTable() {
    PriceTable t;
    t.tickers = {"A", "B"};
    t.dates = {"d1", "d2", "d3", "d4"};
    t.prices = {
        {100.0, 200.0},
        {110.0, 220.0},
        {105.0, 210.0},
        {115.0, 230.0}
    };
    return t;
}

} // namespace

TEST(CovarianceMatrixTest, SymmetricMatrix) {
    ReturnsCalculator calc(makeCorrelatedTable(), 252);
    CovarianceMatrix cov(calc);
    const auto& m = cov.periodic();
    EXPECT_NEAR(m(0, 1), m(1, 0), 1e-12);
}

TEST(CovarianceMatrixTest, PerfectlyCorrelatedAssetsEqualVarianceCovariance) {
    ReturnsCalculator calc(makeCorrelatedTable(), 252);
    CovarianceMatrix cov(calc);
    const auto& m = cov.periodic();
    // Since returns[i][0] == returns[i][1] for all periods, Cov(A,B) == Var(A) == Var(B)
    EXPECT_NEAR(m(0, 0), m(0, 1), 1e-9);
    EXPECT_NEAR(m(1, 1), m(0, 1), 1e-9);
}

TEST(CovarianceMatrixTest, AnnualizationScaling) {
    ReturnsCalculator calc(makeCorrelatedTable(), 252);
    CovarianceMatrix cov(calc);
    const auto& periodic = cov.periodic();
    const auto& annualized = cov.annualized();
    for (std::size_t i = 0; i < 2; ++i)
        for (std::size_t j = 0; j < 2; ++j)
            EXPECT_NEAR(annualized(i, j), periodic(i, j) * 252, 1e-9);
}

TEST(CovarianceMatrixTest, DiagonalIsNonNegative) {
    ReturnsCalculator calc(makeCorrelatedTable(), 252);
    CovarianceMatrix cov(calc);
    const auto& m = cov.periodic();
    EXPECT_GE(m(0, 0), 0.0);
    EXPECT_GE(m(1, 1), 0.0);
}
