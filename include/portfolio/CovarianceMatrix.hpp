#pragma once
#include "portfolio/Matrix.hpp"
#include "portfolio/ReturnsCalculator.hpp"

namespace portfolio {

// Computes the (annualized) sample covariance matrix of asset returns.
class CovarianceMatrix {
public:
    explicit CovarianceMatrix(const ReturnsCalculator& returnsCalc);

    // Annualized covariance matrix (N x N), N = number of assets.
    const Matrix& annualized() const { return annualizedCov_; }

    // Periodic (non-annualized) covariance matrix.
    const Matrix& periodic() const { return periodicCov_; }

private:
    Matrix periodicCov_;
    Matrix annualizedCov_;
};

} // namespace portfolio
