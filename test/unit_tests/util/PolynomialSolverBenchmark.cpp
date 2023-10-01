#include <chrono>
#include <cstdio>
#include <random>

#include <gtest/gtest.h>

#include "util/PolynomialSolver.h"
#include "util/TinyVector.h"

template <class Fun>
static void bench(size_t N, Fun fun) {
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_real_distribution<double> d(-100.0, 100.0);

    size_t n = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned int i = 0; i < N; ++i) {
        auto roots = fun(e, d);
        n += roots.size();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    printf("===== Found %zu roots in %zu iterations and %zu microseconds.\n", n, N, duration.count());
}

TEST(UtilPolynomialSolver, DISABLED_benchmarkQuadratic) {
    bench(10000000, [](std::default_random_engine& e, std::uniform_real_distribution<double>& d) {
        return PolynomialSolver::rootsOfQuadratic(d(e), d(e), d(e), 0.0, 1.0);
    });
}

TEST(UtilPolynomialSolver, DISABLED_benchmarkCubic) {
    bench(10000000, [](std::default_random_engine& e, std::uniform_real_distribution<double>& d) {
        return PolynomialSolver::rootsOfCubic(d(e), d(e), d(e), d(e), 0.0, 1.0);
    });
}

TEST(UtilPolynomialSolver, DISABLED_benchmarkQuartic) {
    bench(10000000, [](std::default_random_engine& e, std::uniform_real_distribution<double>& d) {
        return PolynomialSolver::rootsOfQuartic(d(e), d(e), d(e), d(e), d(e), 0.0, 1.0);
    });
}

TEST(UtilPolynomialSolver, DISABLED_benchmarkQuintic) {
    bench(10000000, [](std::default_random_engine& e, std::uniform_real_distribution<double>& d) {
        PolynomialSolver::PolynomialSolver<5> solver({d(e), d(e), d(e), d(e), d(e), d(e)});
        return solver.findRoots(0.0, 1.0);
    });
}

TEST(UtilPolynomialSolver, DISABLED_benchmarkSextic) {
    bench(10000000, [](std::default_random_engine& e, std::uniform_real_distribution<double>& d) {
        PolynomialSolver::PolynomialSolver<6> solver({d(e), d(e), d(e), d(e), d(e), d(e), d(e)});
        return solver.findRoots(0.0, 1.0);
    });
}
