#pragma once
#include <vector>
#include "portfolio/MarkowitzOptimizer.hpp"

namespace portfolio {

// Generates a series of efficient portfolios spanning a range of target
// returns, tracing out the Markowitz efficient frontier.
class EfficientFrontier {
public:
    EfficientFrontier(const PortfolioMetrics& metrics, std::size_t numPoints = 50);

    // Frontier points ordered by ascending target return.
    const std::vector<PortfolioResult>& points() const { return points_; }

    void exportToCsv(const std::string& filepath) const;

private:
    std::vector<PortfolioResult> points_;
};

} // namespace portfolio
