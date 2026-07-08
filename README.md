# Portfolio Optimizer

A production-quality C++17 implementation of Mean-Variance (Markowitz) Portfolio
Optimization: Minimum Variance Portfolio, Maximum Sharpe Ratio Portfolio, and
the Efficient Frontier, computed from historical stock price data under
**long-only, fully-invested constraints** (no short selling, no leverage).

## Features

- **CSV ingestion** of historical daily closing prices (`CsvReader`)
- **Returns calculation**: simple periodic returns + annualized expected returns (`ReturnsCalculator`)
- **Covariance matrix** estimation, periodic and annualized (`CovarianceMatrix`)
- **Portfolio metrics**: expected return, volatility, Sharpe ratio for any weight vector (`PortfolioMetrics`)
- **Markowitz Optimizer**: constrained numerical (projected gradient descent)
  solutions for:
  - Global Minimum Variance Portfolio (GMVP)
  - Maximum Sharpe Ratio (tangency) Portfolio
  - Efficient portfolio for any target return
  - All subject to `sum(weights) = 1`, `0 <= weight_i <= 1` (long-only, no leverage)
- **Efficient Frontier** generation and CSV export for plotting
- Custom lightweight `Matrix` class (no external linear algebra dependency)
- Modular OOP design, one responsibility per class
- GoogleTest unit test suite (fetched automatically via CMake)

## Project Structure

```
portfolio-optimizer/
├── CMakeLists.txt
├── README.md
├── data/sample_prices.csv
├── include/portfolio/       # public headers
├── src/                     # implementations + main.cpp
└── tests/                   # GoogleTest suite
```

## Build Instructions

Requires CMake >= 3.16 and a C++17 compiler (GCC, Clang, or MSVC). Internet
access is required on first build to fetch GoogleTest via `FetchContent`.

```bash
git clone <repo-url> portfolio-optimizer
cd portfolio-optimizer
mkdir build && cd build
cmake ..
cmake --build . -j
```

To build without tests:

```bash
cmake -DBUILD_TESTS=OFF ..
```

## Running the Optimizer

From the `build` directory:

```bash
./portfolio_optimizer ../data/sample_prices.csv 0.02
```

Arguments (both optional):
1. Path to price CSV (default: `data/sample_prices.csv`)
2. Annual risk-free rate as a decimal (default: `0.02`)

This prints:
- Annualized expected returns per asset
- Minimum Variance Portfolio (weights, return, volatility, Sharpe)
- Maximum Sharpe Ratio Portfolio (weights, return, volatility, Sharpe)
- Exports `efficient_frontier.csv` (50 points: ExpectedReturn, Volatility, SharpeRatio)

## Running Tests

From the `build` directory:

```bash
ctest --output-on-failure
```

or run the test binary directly:

```bash
./tests/portfolio_tests
```

## Input CSV Format

```
Date,TICKER1,TICKER2,...
2024-01-02,185.64,376.04,...
2024-01-03,184.25,370.87,...
...
```

- First column: date (any string format, unused for calculations beyond ordering)
- Remaining columns: closing prices per ticker
- At least 2 rows of prices are required to compute at least one return
- A sample 5-asset (AAPL, MSFT, GOOGL, AMZN, TLT) dataset spanning ~60 trading
  days is provided at `data/sample_prices.csv`

## Methodology Notes

- Returns are **simple returns**: `(P_t - P_{t-1}) / P_{t-1}`
- Expected returns and covariance are annualized assuming **252 trading days/year**
- Covariance uses the **sample covariance** (Bessel's correction, `N-1` denominator)
- All portfolios are **long-only and fully invested**:
  `sum(weights) = 1` and `0 <= weight_i <= 1` for every asset — no short
  selling, no leverage. These constraints define the "capped simplex".
- `MarkowitzOptimizer` solves each problem with **projected gradient
  descent (PGD)** and Armijo backtracking line search:
  - Every gradient step is projected back onto the capped simplex via an
    exact Euclidean projection (bisection on a shift parameter), so weights
    are constrained *during* optimization — never clamped after the fact.
  - **Minimum Variance Portfolio**: minimizes `w'Σw` directly over the
    capped simplex.
  - **Maximum Sharpe Ratio Portfolio**: ascends the (non-concave) Sharpe
    ratio `(w'μ - r_f) / sqrt(w'Σw)` from several starting points (uniform
    weights + each single-asset vertex) to guard against poor local optima.
  - **Efficient Frontier point (target return)**: minimizes `w'Σw` with the
    return constraint `w'μ = target` added as a quadratic penalty, using a
    continuation schedule (penalty weight ramped from `1e2` to `1e8` across
    rounds, warm-starting each round from the previous solution) so the
    constraint is satisfied to high precision while `sum(w)=1` and
    `0<=w<=1` remain exactly enforced throughout.
- A target return is only reachable by a long-only, fully-invested
  portfolio if it lies within `[min(μ), max(μ)]`; `efficientPortfolioForReturn`
  throws `std::runtime_error` outside that range, and `EfficientFrontier`
  skips such infeasible points automatically.

## Extending

- To allow short selling or leverage again, the constraint can be relaxed
  by replacing `projectCappedSimplex` in `MarkowitzOptimizer` with an
  unconstrained normalization (divide by the weight sum) — the rest of the
  PGD machinery (gradients, line search) is unaffected.
- To support log returns instead of simple returns, add an option to
  `ReturnsCalculator`.
- To visualize the efficient frontier, plot `efficient_frontier.csv`
  (Volatility on x-axis, ExpectedReturn on y-axis) using any plotting tool
  (e.g. Python/matplotlib, gnuplot, Excel).

## License

@Mohnishlmao
