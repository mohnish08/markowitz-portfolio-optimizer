#include "portfolio/CsvReader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace portfolio {

std::vector<std::string> CsvReader::splitLine(const std::string& line, char delim) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

PriceTable CsvReader::readPrices(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("CsvReader: unable to open file: " + filepath);
    }

    PriceTable table;
    std::string line;

    if (!std::getline(file, line)) {
        throw std::runtime_error("CsvReader: file is empty: " + filepath);
    }
    auto header = splitLine(line);
    if (header.size() < 2) {
        throw std::runtime_error("CsvReader: header must contain Date + at least one ticker");
    }
    table.tickers.assign(header.begin() + 1, header.end());

    std::size_t lineNo = 1;
    while (std::getline(file, line)) {
        ++lineNo;
        if (line.empty()) continue;
        auto tokens = splitLine(line);
        if (tokens.size() != header.size()) {
            throw std::runtime_error("CsvReader: malformed row at line " + std::to_string(lineNo));
        }
        table.dates.push_back(tokens[0]);
        std::vector<double> row;
        row.reserve(tokens.size() - 1);
        for (std::size_t i = 1; i < tokens.size(); ++i) {
            try {
                row.push_back(std::stod(tokens[i]));
            } catch (const std::exception&) {
                throw std::runtime_error("CsvReader: invalid numeric value at line " + std::to_string(lineNo));
            }
        }
        table.prices.push_back(std::move(row));
    }

    if (table.prices.size() < 2) {
        throw std::runtime_error("CsvReader: need at least 2 rows of prices to compute returns");
    }

    return table;
}

} // namespace portfolio
