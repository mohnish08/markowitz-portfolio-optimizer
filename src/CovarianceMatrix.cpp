#include "portfolio/CovarianceMatrix.hpp"
#include <stdexcept>

namespace portfolio {

CovarianceMatrix::CovarianceMatrix(const ReturnsCalculator& returnsCalc) {
    const auto& returns = returnsCalc.periodicReturns();
    std::size_t n = returnsCalc.numAssets();
    std::size_t t = returns.size();
    if (t < 2) {
        throw std::invalid_argument("CovarianceMatrix: need at least 2 periods of returns");
    }

    auto means = returnsCalc.meanPeriodicReturns();

    periodicCov_ = Matrix(n, n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = i; j < n; ++j) {
            double sum = 0.0;
            for (std::size_t k = 0; k < t; ++k) {
                sum += (returns[k][i] - means[i]) * (returns[k][j] - means[j]);
            }
            double cov = sum / static_cast<double>(t - 1); // sample covariance
            periodicCov_(i, j) = cov;
            periodicCov_(j, i) = cov;
        }
    }

    double periodsPerYear = static_cast<double>(returnsCalc.periodsPerYear());
    annualizedCov_ = periodicCov_ * periodsPerYear;
}

} // namespace portfolio
