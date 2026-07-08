#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "portfolio/CsvReader.hpp"

using portfolio::CsvReader;

namespace {

std::string writeTempCsv(const std::string& name, const std::string& content) {
    std::string path = "test_" + name + ".csv";
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

} // namespace

TEST(CsvReaderTest, ReadsValidPriceTable) {
    std::string path = writeTempCsv("valid",
        "Date,AAPL,MSFT\n"
        "2024-01-01,100.0,200.0\n"
        "2024-01-02,101.0,202.0\n"
        "2024-01-03,102.5,199.0\n");

    auto table = CsvReader::readPrices(path);
    EXPECT_EQ(table.tickers.size(), 2u);
    EXPECT_EQ(table.tickers[0], "AAPL");
    EXPECT_EQ(table.tickers[1], "MSFT");
    ASSERT_EQ(table.dates.size(), 3u);
    EXPECT_EQ(table.dates[0], "2024-01-01");
    EXPECT_DOUBLE_EQ(table.prices[1][1], 202.0);

    std::remove(path.c_str());
}

TEST(CsvReaderTest, MissingFileThrows) {
    EXPECT_THROW(CsvReader::readPrices("nonexistent_file_xyz.csv"), std::runtime_error);
}

TEST(CsvReaderTest, EmptyFileThrows) {
    std::string path = writeTempCsv("empty", "");
    EXPECT_THROW(CsvReader::readPrices(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(CsvReaderTest, MalformedRowThrows) {
    std::string path = writeTempCsv("malformed",
        "Date,AAPL,MSFT\n"
        "2024-01-01,100.0,200.0\n"
        "2024-01-02,101.0\n");
    EXPECT_THROW(CsvReader::readPrices(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(CsvReaderTest, NonNumericValueThrows) {
    std::string path = writeTempCsv("nonnumeric",
        "Date,AAPL\n"
        "2024-01-01,abc\n"
        "2024-01-02,101.0\n");
    EXPECT_THROW(CsvReader::readPrices(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(CsvReaderTest, InsufficientRowsThrows) {
    std::string path = writeTempCsv("onerow",
        "Date,AAPL\n"
        "2024-01-01,100.0\n");
    EXPECT_THROW(CsvReader::readPrices(path), std::runtime_error);
    std::remove(path.c_str());
}

TEST(CsvReaderTest, HeaderOnlyMissingTickerThrows) {
    std::string path = writeTempCsv("headeronly", "Date\n2024-01-01\n");
    EXPECT_THROW(CsvReader::readPrices(path), std::runtime_error);
    std::remove(path.c_str());
}
