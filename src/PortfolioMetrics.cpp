#include "portfolio/PortfolioMetrics.hpp"
#include <cmath>
#include <stdexcept>

namespace portfolio {

PortfolioMetrics::PortfolioMetrics(std::vector<double> expectedReturns, Matrix covariance, double riskFreeRate)
    : expectedReturns_(std::move(expectedReturns)), covariance_(std::move(covariance)), riskFreeRate_(riskFreeRate) {
    std::size_t n = expectedReturns_.size();
    if (covariance_.rows() != n || covariance_.cols() != n) {
        throw std::invalid_argument("PortfolioMetrics: covariance matrix dimensions must match number of assets");
    }
}

void PortfolioMetrics::validateWeights(const std::vector<double>& weights) const {
    if (weights.size() != numAssets()) {
        throw std::invalid_argument("PortfolioMetrics: weights size must match number of assets");
    }
}

double PortfolioMetrics::portfolioReturn(const std::vector<double>& weights) const {
    validateWeights(weights);
    double r = 0.0;
    for (std::size_t i = 0; i < weights.size(); ++i) r += weights[i] * expectedReturns_[i];
    return r;
}

double PortfolioMetrics::portfolioVolatility(const std::vector<double>& weights) const {
    validateWeights(weights);
    Matrix w = Matrix::fromVector(weights);
    Matrix variance = w.transpose() * covariance_ * w;
    double var = variance(0, 0);
    return std::sqrt(std::max(var, 0.0));
}

double PortfolioMetrics::sharpeRatio(const std::vector<double>& weights) const {
    double vol = portfolioVolatility(weights);
    if (vol < 1e-12) return 0.0;
    return (portfolioReturn(weights) - riskFreeRate_) / vol;
}

PortfolioResult PortfolioMetrics::evaluate(const std::vector<double>& weights) const {
    PortfolioResult result;
    result.weights = weights;
    result.expectedReturn = portfolioReturn(weights);
    result.volatility = portfolioVolatility(weights);
    result.sharpeRatio = sharpeRatio(weights);
    return result;
}

} // namespace portfolio
