#include "portfolio/ReturnsCalculator.hpp"
#include <stdexcept>

namespace portfolio {

ReturnsCalculator::ReturnsCalculator(const PriceTable& table, int periodsPerYear)
    : tickers_(table.tickers), periodsPerYear_(periodsPerYear) {
    if (table.prices.size() < 2) {
        throw std::invalid_argument("ReturnsCalculator: need at least 2 price rows");
    }
    numAssets_ = table.tickers.size();

    returns_.reserve(table.prices.size() - 1);
    for (std::size_t i = 1; i < table.prices.size(); ++i) {
        const auto& prev = table.prices[i - 1];
        const auto& curr = table.prices[i];
        std::vector<double> row(numAssets_);
        for (std::size_t j = 0; j < numAssets_; ++j) {
            if (prev[j] <= 0.0) {
                throw std::invalid_argument("ReturnsCalculator: non-positive price encountered");
            }
            row[j] = (curr[j] - prev[j]) / prev[j];
        }
        returns_.push_back(std::move(row));
    }
}

std::vector<double> ReturnsCalculator::meanPeriodicReturns() const {
    std::vector<double> means(numAssets_, 0.0);
    for (const auto& row : returns_) {
        for (std::size_t j = 0; j < numAssets_; ++j) means[j] += row[j];
    }
    for (auto& m : means) m /= static_cast<double>(returns_.size());
    return means;
}

std::vector<double> ReturnsCalculator::annualizedExpectedReturns() const {
    auto means = meanPeriodicReturns();
    for (auto& m : means) m *= periodsPerYear_;
    return means;
}

} // namespace portfolio
