#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>

namespace portfolio {

class Matrix {
public:
    Matrix() = default;
    Matrix(std::size_t rows, std::size_t cols, double init = 0.0);
    explicit Matrix(std::vector<std::vector<double>> data);

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }

    double& operator()(std::size_t r, std::size_t c);
    double operator()(std::size_t r, std::size_t c) const;

    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;
    Matrix operator*(const Matrix& other) const;
    Matrix operator*(double scalar) const;

    Matrix transpose() const;
    Matrix inverse() const;
    double dot(const Matrix& other) const; // for vectors (Nx1)

    static Matrix identity(std::size_t n);
    static Matrix fromVector(const std::vector<double>& v);
    std::vector<double> toVector() const;

private:
    std::size_t rows_ = 0;
    std::size_t cols_ = 0;
    std::vector<double> data_;

    void checkDims(const Matrix& other, bool sameShape) const;
};

} // namespace portfolio
