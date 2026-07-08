#include <gtest/gtest.h>
#include "portfolio/Matrix.hpp"

using portfolio::Matrix;

TEST(MatrixTest, ConstructAndAccess) {
    Matrix m(2, 3, 1.5);
    EXPECT_EQ(m.rows(), 2u);
    EXPECT_EQ(m.cols(), 3u);
    EXPECT_DOUBLE_EQ(m(0, 0), 1.5);
    m(1, 2) = 9.0;
    EXPECT_DOUBLE_EQ(m(1, 2), 9.0);
}

TEST(MatrixTest, FromNestedVector) {
    Matrix m({{1, 2}, {3, 4}});
    EXPECT_DOUBLE_EQ(m(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(m(1, 0), 3.0);
}

TEST(MatrixTest, RaggedRowsThrows) {
    EXPECT_THROW(Matrix({{1, 2}, {3}}), std::invalid_argument);
}

TEST(MatrixTest, Addition) {
    Matrix a({{1, 2}, {3, 4}});
    Matrix b({{5, 6}, {7, 8}});
    Matrix c = a + b;
    EXPECT_DOUBLE_EQ(c(0, 0), 6.0);
    EXPECT_DOUBLE_EQ(c(1, 1), 12.0);
}

TEST(MatrixTest, Subtraction) {
    Matrix a({{5, 6}, {7, 8}});
    Matrix b({{1, 2}, {3, 4}});
    Matrix c = a - b;
    EXPECT_DOUBLE_EQ(c(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(c(1, 1), 4.0);
}

TEST(MatrixTest, Multiplication) {
    Matrix a({{1, 2}, {3, 4}});
    Matrix b({{5, 6}, {7, 8}});
    Matrix c = a * b;
    EXPECT_DOUBLE_EQ(c(0, 0), 19.0);
    EXPECT_DOUBLE_EQ(c(0, 1), 22.0);
    EXPECT_DOUBLE_EQ(c(1, 0), 43.0);
    EXPECT_DOUBLE_EQ(c(1, 1), 50.0);
}

TEST(MatrixTest, ScalarMultiplication) {
    Matrix a({{1, 2}, {3, 4}});
    Matrix c = a * 2.0;
    EXPECT_DOUBLE_EQ(c(0, 0), 2.0);
    EXPECT_DOUBLE_EQ(c(1, 1), 8.0);
}

TEST(MatrixTest, Transpose) {
    Matrix a({{1, 2, 3}, {4, 5, 6}});
    Matrix t = a.transpose();
    EXPECT_EQ(t.rows(), 3u);
    EXPECT_EQ(t.cols(), 2u);
    EXPECT_DOUBLE_EQ(t(2, 0), 3.0);
    EXPECT_DOUBLE_EQ(t(0, 1), 4.0);
}

TEST(MatrixTest, Identity) {
    Matrix id = Matrix::identity(3);
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            EXPECT_DOUBLE_EQ(id(i, j), (i == j) ? 1.0 : 0.0);
}

TEST(MatrixTest, InverseOfIdentityIsIdentity) {
    Matrix id = Matrix::identity(3);
    Matrix inv = id.inverse();
    for (std::size_t i = 0; i < 3; ++i)
        for (std::size_t j = 0; j < 3; ++j)
            EXPECT_NEAR(inv(i, j), id(i, j), 1e-9);
}

TEST(MatrixTest, InverseCorrectness) {
    Matrix a({{4, 7}, {2, 6}});
    Matrix inv = a.inverse();
    Matrix product = a * inv;
    Matrix id = Matrix::identity(2);
    for (std::size_t i = 0; i < 2; ++i)
        for (std::size_t j = 0; j < 2; ++j)
            EXPECT_NEAR(product(i, j), id(i, j), 1e-9);
}

TEST(MatrixTest, SingularMatrixThrows) {
    Matrix singular({{1, 2}, {2, 4}});
    EXPECT_THROW(singular.inverse(), std::runtime_error);
}

TEST(MatrixTest, DotProduct) {
    Matrix a = Matrix::fromVector({1, 2, 3});
    Matrix b = Matrix::fromVector({4, 5, 6});
    EXPECT_DOUBLE_EQ(a.dot(b), 32.0);
}

TEST(MatrixTest, VectorRoundTrip) {
    std::vector<double> v = {1.0, 2.0, 3.0};
    Matrix m = Matrix::fromVector(v);
    auto back = m.toVector();
    ASSERT_EQ(back.size(), v.size());
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_DOUBLE_EQ(back[i], v[i]);
}

TEST(MatrixTest, OutOfRangeAccessThrows) {
    Matrix m(2, 2);
    EXPECT_THROW(m(5, 0), std::out_of_range);
}
