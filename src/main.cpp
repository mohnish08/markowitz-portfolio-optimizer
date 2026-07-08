#include <iostream>
#include <iomanip>
#include <string>
#include "portfolio/CsvReader.hpp"
#include "portfolio/ReturnsCalculator.hpp"
#include "portfolio/CovarianceMatrix.hpp"
#include "portfolio/PortfolioMetrics.hpp"
#include "portfolio/MarkowitzOptimizer.hpp"
#include "portfolio/EfficientFrontier.hpp"

using namespace portfolio;

namespace {

void printResult(const std::string& label, const PortfolioResult& r, const std::vector<std::string>& tickers) {
    std::cout << "\n=== " << label << " ===\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Expected Return: " << r.expectedReturn * 100 << "%\n";
    std::cout << "Volatility:      " << r.volatility * 100 << "%\n";
    std::cout << "Sharpe Ratio:    " << r.sharpeRatio << "\n";
    std::cout << "Weights:\n";
    for (std::size_t i = 0; i < r.weights.size() && i < tickers.size(); ++i) {
        std::cout << "  " << tickers[i] << ": " << r.weights[i] * 100 << "%\n";
    }
}

} // namespace

int main(int argc, char** argv) {
    std::string csvPath = argc > 1 ? argv[1] : "data/sample_prices.csv";
    double riskFreeRate = argc > 2 ? std::stod(argv[2]) : 0.02;

    try {
        std::cout << "Loading price data from: " << csvPath << "\n";
        PriceTable table = CsvReader::readPrices(csvPath);
        std::cout << "Loaded " << table.dates.size() << " rows for "
                   << table.tickers.size() << " assets.\n";

        ReturnsCalculator returnsCalc(table, 252);
        CovarianceMatrix covMatrix(returnsCalc);
        PortfolioMetrics metrics(returnsCalc.annualizedExpectedReturns(), covMatrix.annualized(), riskFreeRate);

        std::cout << "\nAnnualized Expected Returns:\n";
        const auto& mu = metrics.expectedReturns();
        for (std::size_t i = 0; i < table.tickers.size(); ++i) {
            std::cout << "  " << table.tickers[i] << ": " << std::fixed << std::setprecision(2)
                       << mu[i] * 100 << "%\n";
        }

        MarkowitzOptimizer optimizer(metrics);

        auto minVar = optimizer.minimumVariancePortfolio();
        printResult("Minimum Variance Portfolio", minVar, table.tickers);

        auto maxSharpe = optimizer.maximumSharpePortfolio();
        printResult("Maximum Sharpe Ratio Portfolio", maxSharpe, table.tickers);

        EfficientFrontier frontier(metrics, 50);
        std::string frontierPath = "efficient_frontier.csv";
        frontier.exportToCsv(frontierPath);
        std::cout << "\nEfficient frontier (" << frontier.points().size()
                   << " points) exported to: " << frontierPath << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
