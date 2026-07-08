#include "portfolio/EfficientFrontier.hpp"
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace portfolio {

EfficientFrontier::EfficientFrontier(const PortfolioMetrics& metrics, std::size_t numPoints) {
    if (numPoints < 2) {
        throw std::invalid_argument("EfficientFrontier: numPoints must be >= 2");
    }

    MarkowitzOptimizer optimizer(metrics);
    const auto& mu = metrics.expectedReturns();
    double minRet = *std::min_element(mu.begin(), mu.end());
    double maxRet = *std::max_element(mu.begin(), mu.end());

    // Expand range slightly to cover portfolios beyond individual asset extremes.
    double span = maxRet - minRet;
    double lo = minRet - 0.25 * span;
    double hi = maxRet + 0.25 * span;
    if (span < 1e-9) { lo = minRet - 0.05; hi = maxRet + 0.05; }

    points_.reserve(numPoints);
    for (std::size_t i = 0; i < numPoints; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(numPoints - 1);
        double targetReturn = lo + t * (hi - lo);
        try {
            points_.push_back(optimizer.efficientPortfolioForReturn(targetReturn));
        } catch (const std::runtime_error&) {
            // Skip degenerate points (e.g. singular system); continue frontier generation.
            continue;
        }
    }

    std::sort(points_.begin(), points_.end(), [](const PortfolioResult& a, const PortfolioResult& b) {
        return a.expectedReturn < b.expectedReturn;
    });
}

void EfficientFrontier::exportToCsv(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("EfficientFrontier: unable to open file for writing: " + filepath);
    }
    file << "ExpectedReturn,Volatility,SharpeRatio\n";
    for (const auto& p : points_) {
        file << p.expectedReturn << "," << p.volatility << "," << p.sharpeRatio << "\n";
    }
}

} // namespace portfolio
