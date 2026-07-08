#pragma once
#include <vector>
#include "portfolio/CsvReader.hpp"

namespace portfolio {

// Computes periodic and annualized returns from a PriceTable.
class ReturnsCalculator {
public:
    explicit ReturnsCalculator(const PriceTable& table, int periodsPerYear = 252);

    // returns_[i][j] = simple return of asset j between date i and i+1
    const std::vector<std::vector<double>>& periodicReturns() const { return returns_; }

    // Mean periodic return per asset, annualized by periodsPerYear.
    std::vector<double> annualizedExpectedReturns() const;

    // Mean periodic return per asset (not annualized).
    std::vector<double> meanPeriodicReturns() const;

    std::size_t numAssets() const { return numAssets_; }
    std::size_t numPeriods() const { return returns_.size(); }
    int periodsPerYear() const { return periodsPerYear_; }
    const std::vector<std::string>& tickers() const { return tickers_; }

private:
    std::vector<std::vector<double>> returns_;
    std::vector<std::string> tickers_;
    std::size_t numAssets_ = 0;
    int periodsPerYear_;
};

} // namespace portfolio
