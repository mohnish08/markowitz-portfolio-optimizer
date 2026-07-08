#pragma once
#include <vector>
#include "portfolio/Matrix.hpp"

namespace portfolio {

struct PortfolioResult {
    std::vector<double> weights;
    double expectedReturn = 0.0;
    double volatility = 0.0;
    double sharpeRatio = 0.0;
};

// Computes return, volatility, and Sharpe ratio for a given weight vector.
class PortfolioMetrics {
public:
    PortfolioMetrics(std::vector<double> expectedReturns, Matrix covariance, double riskFreeRate = 0.0);

    double portfolioReturn(const std::vector<double>& weights) const;
    double portfolioVolatility(const std::vector<double>& weights) const;
    double sharpeRatio(const std::vector<double>& weights) const;

    PortfolioResult evaluate(const std::vector<double>& weights) const;

    std::size_t numAssets() const { return expectedReturns_.size(); }
    double riskFreeRate() const { return riskFreeRate_; }
    const std::vector<double>& expectedReturns() const { return expectedReturns_; }
    const Matrix& covariance() const { return covariance_; }

private:
    std::vector<double> expectedReturns_;
    Matrix covariance_;
    double riskFreeRate_;

    void validateWeights(const std::vector<double>& weights) const;
};

} // namespace portfolio
