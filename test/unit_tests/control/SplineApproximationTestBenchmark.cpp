/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <chrono>
#include <random>
#include <vector>

#include <gtest/gtest.h>
#include <math.h>
using namespace std::chrono;

#include "control/tools/splineapproximation/SplineApproximatorLive.h"
#include "control/tools/splineapproximation/SplineApproximatorSchneider.h"
#include "model/MathVect.h"
#include "model/Point.h"

// Depend on the random path generator below. Those constant could need adjusting if getRandomPath is changed.
//   = CompressedSize / InputSize
static constexpr double COMPRESSION_REQUIREMENT_POST = 0.5;
static constexpr double COMPRESSION_REQUIREMENT_LIVE = 0.38;

/**
 * Create a random path that looks like a path one could make with a stylus. Not perfect, though.
 */
static std::vector<Point> getRandomPath(size_t nbPts) {
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_real_distribution<double> angle(-M_PI, M_PI);
    std::normal_distribution<double> normalD(0.0, 1.0);

    std::vector<Point> pts;
    pts.reserve(nbPts);
    pts.emplace_back(0.0, 0.0, 1.0);
    double a = angle(e);
    pts.emplace_back(cos(a), sin(a), 1.0 + 0.1 * normalD(e));

    auto beforeLast = pts.begin();
    auto last = std::prev(pts.end());
    while (pts.size() != nbPts) {
        MathVect2 v(*beforeLast, *last);
        double b = angle(e);
        double c = 0.3 * normalD(e);
        v = v + MathVect2(c * cos(b), c * sin(b));
        double dp = normalD(e);
        double m = std::min(0.5, dp > 0 ? 2.0 - last->z : last->z);
        dp *= 0.2 * m;
        pts.emplace_back(last->x + v.dx, last->y + v.dy, last->z + dp);
        beforeLast = last++;
    }
    return pts;
}

template <typename Fun>
static double runBench(size_t nb_iteration, size_t nb_pts_per_path, Fun fun) {
    size_t nbSeg = 0;
    auto start = high_resolution_clock::now();
    for (size_t n = 0; n < nb_iteration; ++n) {
        auto path = getRandomPath(nb_pts_per_path);
        nbSeg += fun(path);
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    double compression = double(nbSeg * 3 + nb_iteration) / double(nb_pts_per_path * nb_iteration);
    printf("===== Approximated %zu points by %zu spline segments in %zu microseconds. (data saving rate %5.2f%%)\n",
           nb_pts_per_path * nb_iteration, nbSeg, duration.count(), 100.0 * (1.0 - compression));
    return compression;
}

static size_t runPostApproximation(const std::vector<Point>& path) {
    SplineApproximator::Schneider approximator(path);
    approximator.run();
    return approximator.getSpline().nbSegments();
}

static size_t runLiveApproximation(const std::vector<Point>& path) {
    auto it = path.begin();
    auto spline = std::make_shared<Spline>(*it++);
    SplineApproximator::Live approximator(spline);
    for (; it != path.end(); ++it) {
        approximator.feedPoint(*it);
    }
    approximator.finalize();
    return spline->nbSegments();
}

TEST(SplineApproximationTest, testPostApproximation) {
    double compression = runBench(1000, 300, runPostApproximation);
    EXPECT_TRUE(compression < COMPRESSION_REQUIREMENT_POST);
}

TEST(SplineApproximationTest, DISABLED_benchmarkPostApproximation) {
    double compression = runBench(10000, 600, runPostApproximation);
    EXPECT_TRUE(compression < COMPRESSION_REQUIREMENT_POST);
}

TEST(SplineApproximationTest, testLiveApproximation) {
    double compression = runBench(1000, 300, runLiveApproximation);
    EXPECT_TRUE(compression < COMPRESSION_REQUIREMENT_LIVE);
}

TEST(SplineApproximationTest, DISABLED_benchmarkLiveApproximation) {
    double compression = runBench(10000, 600, runLiveApproximation);
    EXPECT_TRUE(compression < COMPRESSION_REQUIREMENT_LIVE);
}
