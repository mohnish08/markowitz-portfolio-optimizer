#include "portfolio/MarkowitzOptimizer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace portfolio {

MarkowitzOptimizer::MarkowitzOptimizer(const PortfolioMetrics& metrics) : metrics_(metrics) {}

// --- Projection onto the capped simplex {sum(w)=1, 0<=w<=1} ---------------
//
// Standard bisection on a shift parameter tau: setting w_i = clip(v_i - tau,
// 0, 1), the function f(tau) = sum_i w_i is continuous and monotonically
// non-increasing in tau. At tau = min(v) - 1, every w_i = 1 (sum = n >= 1);
// at tau = max(v), every w_i = 0 (sum = 0 <= 1). So a unique tau with
// f(tau) = 1 exists in between, and bisection finds it to machine precision.
// This is an exact Euclidean projection, not a post-hoc clamp: it is the
// closest feasible point (in L2 distance) to the unprojected vector v.
std::vector<double> MarkowitzOptimizer::projectCappedSimplex(const std::vector<double>& v) {
    std::size_t n = v.size();
    if (n == 0) return {};
    if (n == 1) return {1.0}; // only one feasible point in the capped simplex

    double lo = *std::min_element(v.begin(), v.end()) - 1.0;
    double hi = *std::max_element(v.begin(), v.end());

    auto sumAt = [&](double tau) {
        double s = 0.0;
        for (double vi : v) s += std::clamp(vi - tau, 0.0, 1.0);
        return s;
    };

    for (int iter = 0; iter < 100; ++iter) {
        double mid = 0.5 * (lo + hi);
        // Increasing tau shrinks every clipped term, so sum(tau) decreases.
        if (sumAt(mid) > 1.0) lo = mid; else hi = mid;
        if (hi - lo < 1e-15) break;
    }

    double tau = 0.5 * (lo + hi);
    std::vector<double> w(n);
    for (std::size_t i = 0; i < n; ++i) w[i] = std::clamp(v[i] - tau, 0.0, 1.0);
    return w;
}

// --- Generic projected gradient descent/ascent with backtracking ----------
//
// At each iteration: take a trial step along +/- gradient, project onto the
// capped simplex, and check whether the objective actually improved. If not,
// halve the step and retry (Armijo-style backtracking). This guarantees
// monotonic improvement of the objective on every accepted iteration and
// naturally handles the non-smooth "kinks" introduced by projection, without
// requiring a hand-tuned learning rate or Lipschitz-constant estimate.
std::vector<double> MarkowitzOptimizer::projectedGradientOptimize(
    std::vector<double> w0,
    const std::function<std::pair<double, std::vector<double>>(const std::vector<double>&)>& evalGrad,
    bool maximize,
    int maxIterations) const {

    std::vector<double> w = projectCappedSimplex(w0);
    double step = 1.0;

    for (int iter = 0; iter < maxIterations; ++iter) {
        auto [fVal, grad] = evalGrad(w);

        std::vector<double> wNext = w;
        double trialStep = step;
        bool improved = false;

        for (int attempt = 0; attempt < 40; ++attempt) {
            std::vector<double> candidate(w.size());
            for (std::size_t i = 0; i < w.size(); ++i) {
                double direction = maximize ? grad[i] : -grad[i];
                candidate[i] = w[i] + trialStep * direction;
            }
            std::vector<double> projected = projectCappedSimplex(candidate);
            double fNext = evalGrad(projected).first;

            bool isBetter = maximize ? (fNext > fVal + 1e-15) : (fNext < fVal - 1e-15);
            if (isBetter) {
                wNext = projected;
                improved = true;
                break;
            }
            trialStep *= 0.5;
        }

        if (!improved) break; // no improving feasible step found: converged

        double delta = 0.0;
        for (std::size_t i = 0; i < w.size(); ++i) delta += std::fabs(wNext[i] - w[i]);

        w = wNext;
        step = std::min(trialStep * 2.0, 1.0); // grow step again next round

        if (delta < 1e-13) break;
    }

    return w;
}

PortfolioResult MarkowitzOptimizer::minimumVariancePortfolio() const {
    std::size_t n = metrics_.numAssets();
    const Matrix& cov = metrics_.covariance();

    // Objective: minimize w'Sigma*w. Gradient: 2*Sigma*w.
    auto evalGrad = [&](const std::vector<double>& w) -> std::pair<double, std::vector<double>> {
        Matrix wMat = Matrix::fromVector(w);
        Matrix sigmaW = cov * wMat;
        double variance = (wMat.transpose() * sigmaW)(0, 0);
        return {variance, (sigmaW * 2.0).toVector()};
    };

    std::vector<double> w0(n, 1.0 / static_cast<double>(n)); // uniform start
    auto w = projectedGradientOptimize(w0, evalGrad, /*maximize=*/false);
    return metrics_.evaluate(w);
}

PortfolioResult MarkowitzOptimizer::maximumSharpePortfolio() const {
    std::size_t n = metrics_.numAssets();
    const Matrix& cov = metrics_.covariance();
    const auto& mu = metrics_.expectedReturns();
    double rf = metrics_.riskFreeRate();

    std::vector<double> excess(n);
    for (std::size_t i = 0; i < n; ++i) excess[i] = mu[i] - rf;

    // Objective: maximize Sharpe(w) = (w.excess) / sqrt(w'Sigma*w).
    // Gradient: d/dw_i [R/sqrt(V)] = excess_i/sqrt(V) - R*(Sigma*w)_i / V^1.5
    auto evalGrad = [&](const std::vector<double>& w) -> std::pair<double, std::vector<double>> {
        Matrix wMat = Matrix::fromVector(w);
        Matrix sigmaW = cov * wMat;
        double variance = (wMat.transpose() * sigmaW)(0, 0);
        double portReturn = std::inner_product(w.begin(), w.end(), excess.begin(), 0.0);

        double vol = std::sqrt(std::max(variance, 1e-18));
        double sharpe = portReturn / vol;
        double volCubed = vol * vol * vol;

        std::vector<double> grad(n);
        for (std::size_t i = 0; i < n; ++i) {
            grad[i] = excess[i] / vol - portReturn * sigmaW(i, 0) / volCubed;
        }
        return {sharpe, grad};
    };

    // The Sharpe ratio is not concave over the simplex in general, so guard
    // against poor local optima by ascending from several starting points:
    // the uniform portfolio and each single-asset vertex.
    std::vector<std::vector<double>> starts;
    starts.push_back(std::vector<double>(n, 1.0 / static_cast<double>(n)));
    for (std::size_t i = 0; i < n; ++i) {
        std::vector<double> vertex(n, 0.0);
        vertex[i] = 1.0;
        starts.push_back(vertex);
    }

    std::vector<double> best;
    double bestSharpe = -std::numeric_limits<double>::infinity();
    for (const auto& start : starts) {
        auto w = projectedGradientOptimize(start, evalGrad, /*maximize=*/true);
        double s = evalGrad(w).first;
        if (s > bestSharpe) {
            bestSharpe = s;
            best = w;
        }
    }

    return metrics_.evaluate(best);
}

PortfolioResult MarkowitzOptimizer::efficientPortfolioForReturn(double targetReturn) const {
    std::size_t n = metrics_.numAssets();
    const Matrix& cov = metrics_.covariance();
    const auto& mu = metrics_.expectedReturns();

    double minMu = *std::min_element(mu.begin(), mu.end());
    double maxMu = *std::max_element(mu.begin(), mu.end());
    if (targetReturn < minMu - 1e-9 || targetReturn > maxMu + 1e-9) {
        throw std::runtime_error(
            "MarkowitzOptimizer: target return is infeasible under long-only, fully-invested constraints");
    }

    // The equality constraint mu.w == targetReturn cannot be enforced by a
    // simple bisection projection alongside the capped-simplex constraints,
    // so it is instead added as a quadratic penalty term to the variance
    // objective: minimize w'Sigma*w + rho*(mu.w - targetReturn)^2, while
    // sum(w)=1 and 0<=w<=1 remain exactly enforced via projection every
    // iteration. A continuation schedule ramps rho up across several
    // rounds (warm-starting each round from the previous solution), so the
    // return constraint is satisfied to high precision without the
    // instability of starting directly with a very large penalty.
    std::vector<double> w(n, 1.0 / static_cast<double>(n));

    for (double rho : {1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8}) {
        auto evalGrad = [&](const std::vector<double>& weights) -> std::pair<double, std::vector<double>> {
            Matrix wMat = Matrix::fromVector(weights);
            Matrix sigmaW = cov * wMat;
            double variance = (wMat.transpose() * sigmaW)(0, 0);

            double portReturn = std::inner_product(weights.begin(), weights.end(), mu.begin(), 0.0);
            double residual = portReturn - targetReturn;

            double objective = variance + rho * residual * residual;
            std::vector<double> grad(n);
            for (std::size_t i = 0; i < n; ++i) {
                grad[i] = 2.0 * sigmaW(i, 0) + 2.0 * rho * residual * mu[i];
            }
            return {objective, grad};
        };
        w = projectedGradientOptimize(w, evalGrad, /*maximize=*/false, 1500);
    }

    return metrics_.evaluate(w);
}

} // namespace portfolio
