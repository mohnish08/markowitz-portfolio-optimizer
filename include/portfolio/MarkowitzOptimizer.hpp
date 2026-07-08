#pragma once
#include "portfolio/PortfolioMetrics.hpp"
#include <functional>
#include <utility>

namespace portfolio {

// Solves for long-only, fully-invested Markowitz portfolios using a
// projected gradient descent (PGD) numerical optimizer with Armijo
// backtracking line search.
//
// Every portfolio returned satisfies, by construction of the optimizer
// (not by post-hoc clamping):
//   - sum(weights) == 1        (fully invested, no leverage)
//   - 0 <= weight_i <= 1       (no short selling, no per-asset leverage)
//
// These two constraints define the "capped simplex". At every iteration,
// the gradient step is projected back onto the capped simplex via an exact
// Euclidean projection (bisection on a shift parameter), so infeasible
// weights are never produced or clamped after the fact -- feasibility is
// maintained throughout the optimization trajectory.
class MarkowitzOptimizer {
public:
    explicit MarkowitzOptimizer(const PortfolioMetrics& metrics);

    // Global Minimum Variance Portfolio (GMVP) under long-only constraints.
    PortfolioResult minimumVariancePortfolio() const;

    // Maximum Sharpe Ratio (tangency) Portfolio under long-only constraints.
    // The Sharpe ratio is not concave in general, so the ascent is repeated
    // from several starting points (uniform + each single-asset vertex) and
    // the best local optimum found is returned.
    PortfolioResult maximumSharpePortfolio() const;

    // Minimum-variance portfolio subject to E[R] = targetReturn, in
    // addition to the long-only capped-simplex constraints. Throws
    // std::runtime_error if targetReturn is infeasible, i.e. outside
    // [min(mu), max(mu)] (the only returns reachable by a long-only,
    // fully-invested portfolio).
    PortfolioResult efficientPortfolioForReturn(double targetReturn) const;

private:
    const PortfolioMetrics& metrics_;

    // Exact Euclidean projection onto the capped simplex
    // {w : sum(w) = 1, 0 <= w_i <= 1}, via bisection on a shift parameter
    // tau such that w_i = clip(v_i - tau, 0, 1) sums to exactly 1.
    static std::vector<double> projectCappedSimplex(const std::vector<double>& v);

    // Generic projected gradient optimizer with Armijo backtracking line
    // search. `evalGrad` returns {objectiveValue, gradient} at a point w.
    // `maximize` selects ascent (Sharpe ratio) vs descent (variance).
    // Every iterate is projected onto the capped simplex, so the returned
    // weights are always feasible.
    std::vector<double> projectedGradientOptimize(
        std::vector<double> w0,
        const std::function<std::pair<double, std::vector<double>>(const std::vector<double>&)>& evalGrad,
        bool maximize,
        int maxIterations = 3000) const;
};

} // namespace portfolio
