#include "portfolio/Matrix.hpp"
#include <algorithm>
#include <cmath>

namespace portfolio {

Matrix::Matrix(std::size_t rows, std::size_t cols, double init)
    : rows_(rows), cols_(cols), data_(rows * cols, init) {}

Matrix::Matrix(std::vector<std::vector<double>> data) {
    rows_ = data.size();
    cols_ = rows_ > 0 ? data[0].size() : 0;
    data_.reserve(rows_ * cols_);
    for (const auto& row : data) {
        if (row.size() != cols_) {
            throw std::invalid_argument("Matrix: ragged row sizes");
        }
        data_.insert(data_.end(), row.begin(), row.end());
    }
}

double& Matrix::operator()(std::size_t r, std::size_t c) {
    if (r >= rows_ || c >= cols_) throw std::out_of_range("Matrix index out of range");
    return data_[r * cols_ + c];
}

double Matrix::operator()(std::size_t r, std::size_t c) const {
    if (r >= rows_ || c >= cols_) throw std::out_of_range("Matrix index out of range");
    return data_[r * cols_ + c];
}

void Matrix::checkDims(const Matrix& other, bool sameShape) const {
    if (sameShape) {
        if (rows_ != other.rows_ || cols_ != other.cols_)
            throw std::invalid_argument("Matrix: dimension mismatch");
    } else {
        if (cols_ != other.rows_)
            throw std::invalid_argument("Matrix: incompatible dimensions for multiply");
    }
}

Matrix Matrix::operator+(const Matrix& other) const {
    checkDims(other, true);
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] + other.data_[i];
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    checkDims(other, true);
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] - other.data_[i];
    return result;
}

Matrix Matrix::operator*(const Matrix& other) const {
    checkDims(other, false);
    Matrix result(rows_, other.cols_, 0.0);
    for (std::size_t i = 0; i < rows_; ++i) {
        for (std::size_t k = 0; k < cols_; ++k) {
            double a = (*this)(i, k);
            if (a == 0.0) continue;
            for (std::size_t j = 0; j < other.cols_; ++j) {
                result(i, j) += a * other(k, j);
            }
        }
    }
    return result;
}

Matrix Matrix::operator*(double scalar) const {
    Matrix result(rows_, cols_);
    for (std::size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] * scalar;
    return result;
}

Matrix Matrix::transpose() const {
    Matrix result(cols_, rows_);
    for (std::size_t i = 0; i < rows_; ++i)
        for (std::size_t j = 0; j < cols_; ++j)
            result(j, i) = (*this)(i, j);
    return result;
}

Matrix Matrix::identity(std::size_t n) {
    Matrix m(n, n, 0.0);
    for (std::size_t i = 0; i < n; ++i) m(i, i) = 1.0;
    return m;
}

Matrix Matrix::inverse() const {
    if (rows_ != cols_) throw std::invalid_argument("Matrix: inverse requires square matrix");
    std::size_t n = rows_;
    Matrix aug(n, 2 * n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) aug(i, j) = (*this)(i, j);
        aug(i, n + i) = 1.0;
    }

    for (std::size_t col = 0; col < n; ++col) {
        std::size_t pivotRow = col;
        double maxVal = std::fabs(aug(col, col));
        for (std::size_t r = col + 1; r < n; ++r) {
            if (std::fabs(aug(r, col)) > maxVal) {
                maxVal = std::fabs(aug(r, col));
                pivotRow = r;
            }
        }
        if (maxVal < 1e-12) throw std::runtime_error("Matrix: singular matrix, cannot invert");

        if (pivotRow != col) {
            for (std::size_t j = 0; j < 2 * n; ++j) std::swap(aug(col, j), aug(pivotRow, j));
        }

        double pivot = aug(col, col);
        for (std::size_t j = 0; j < 2 * n; ++j) aug(col, j) /= pivot;

        for (std::size_t r = 0; r < n; ++r) {
            if (r == col) continue;
            double factor = aug(r, col);
            if (factor == 0.0) continue;
            for (std::size_t j = 0; j < 2 * n; ++j) aug(r, j) -= factor * aug(col, j);
        }
    }

    Matrix result(n, n);
    for (std::size_t i = 0; i < n; ++i)
        for (std::size_t j = 0; j < n; ++j)
            result(i, j) = aug(i, n + j);
    return result;
}

double Matrix::dot(const Matrix& other) const {
    if (cols_ != 1 || other.cols_ != 1 || rows_ != other.rows_)
        throw std::invalid_argument("Matrix: dot requires equal-length column vectors");
    double sum = 0.0;
    for (std::size_t i = 0; i < rows_; ++i) sum += (*this)(i, 0) * other(i, 0);
    return sum;
}

Matrix Matrix::fromVector(const std::vector<double>& v) {
    Matrix m(v.size(), 1);
    for (std::size_t i = 0; i < v.size(); ++i) m(i, 0) = v[i];
    return m;
}

std::vector<double> Matrix::toVector() const {
    std::vector<double> v;
    v.reserve(rows_ * cols_);
    for (std::size_t i = 0; i < rows_; ++i)
        for (std::size_t j = 0; j < cols_; ++j)
            v.push_back((*this)(i, j));
    return v;
}

} // namespace portfolio
