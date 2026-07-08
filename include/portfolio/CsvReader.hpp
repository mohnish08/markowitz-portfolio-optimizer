#pragma once
#include <string>
#include <vector>
#include <map>

namespace portfolio {

// Represents historical price data: dates + one price series per ticker.
struct PriceTable {
    std::vector<std::string> dates;
    std::vector<std::string> tickers;
    // prices[i][j] = price of tickers[j] on dates[i]
    std::vector<std::vector<double>> prices;
};

class CsvReader {
public:
    // Reads a CSV with header: Date,TICKER1,TICKER2,...
    // Each subsequent row: YYYY-MM-DD,price1,price2,...
    static PriceTable readPrices(const std::string& filepath);

private:
    static std::vector<std::string> splitLine(const std::string& line, char delim = ',');
};

} // namespace portfolio
