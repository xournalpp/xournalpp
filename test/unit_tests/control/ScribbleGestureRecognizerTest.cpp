#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "control/tools/ScribbleGestureRecognizer.h"
#include "model/Point.h"

namespace {
using Path = std::vector<Point>;

auto makeScribble(size_t passes = 4) -> Path {
    Path points;
    for (size_t i = 0; i <= passes; ++i) {
        points.emplace_back(i % 2 == 0 ? 0.0 : 80.0, 2.0 * static_cast<double>(i));
    }
    return points;
}

auto transform(std::span<const Point> points, double angle, double scale, double offsetX, double offsetY) -> Path {
    Path result;
    for (const Point& point: points) {
        result.emplace_back(offsetX + scale * (point.x * std::cos(angle) - point.y * std::sin(angle)),
                            offsetY + scale * (point.x * std::sin(angle) + point.y * std::cos(angle)), point.z);
    }
    return result;
}

auto subdivide(std::span<const Point> points) -> Path {
    Path result{points.front()};
    for (size_t i = 1; i < points.size(); ++i) {
        const size_t steps = i % 2 == 0 ? 3 : 100;
        for (size_t j = 1; j <= steps; ++j) {
            const double t = static_cast<double>(j) / static_cast<double>(steps);
            result.emplace_back(std::lerp(points[i - 1].x, points[i].x, t), std::lerp(points[i - 1].y, points[i].y, t));
        }
    }
    return result;
}

auto ordinaryDrawings() -> std::vector<Path> {
    std::vector<Path> paths{
            {{0, 0}, {80, 0}},
            {{0, 0}, {80, 1}, {0, 2}},
            {{0, 0}, {80, 1}, {0, 2}, {80, 3}},
            {{0, 0}, {80, 80}, {40, 40}, {80, 0}, {0, 80}},
            {{0, 0}, {20, 50}, {40, 0}, {60, 50}, {80, 0}},
            {{0, 50}, {0, 0}, {40, 40}, {80, 0}, {80, 50}},
            {{0, 20}, {80, 20}, {60, 0}, {80, 20}, {60, 40}},
            {{0, 0}, {80, 0}, {0, 20}, {80, 20}, {0, 40}, {80, 40}, {0, 60}},
            {{0, 40}, {20, 40}, {20, 0}, {60, 10}},
            {{0, 20}, {40, 20}, {20, 20}, {20, 0}, {20, 40}},
    };
    Path circle;
    Path spiral;
    Path cursive;
    for (size_t i = 0; i <= 128; ++i) {
        const double t = static_cast<double>(i) / 128.0;
        const double angle = 2.0 * std::numbers::pi * t;
        circle.emplace_back(40.0 * std::cos(angle), 40.0 * std::sin(angle));
        spiral.emplace_back((5.0 + 35.0 * t) * std::cos(3.0 * angle), (5.0 + 35.0 * t) * std::sin(3.0 * angle));
        cursive.emplace_back(80.0 * t, 20.0 * std::sin(4.0 * angle));
    }
    paths.push_back(circle);
    paths.push_back(spiral);
    paths.push_back(cursive);
    return paths;
}
}  // namespace

TEST(ScribbleGestureRecognizer, recognizesRepeatedFullPasses) {
    for (size_t passes: {4U, 6U, 8U}) {
        SCOPED_TRACE(passes);
        EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(makeScribble(passes)));
    }
}

TEST(ScribbleGestureRecognizer, preservesDecisionAcrossOrientationScaleAndTranslation) {
    for (double angle: {0.0, std::numbers::pi / 6.0, std::numbers::pi / 2.0, 2.7}) {
        for (double scale: {0.25, 1.0, 4.0}) {
            SCOPED_TRACE(::testing::Message() << angle << ", " << scale);
            EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(transform(makeScribble(), angle, scale, 120, -70)));
            for (const Path& drawing: ordinaryDrawings()) {
                EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(transform(drawing, angle, scale, 120, -70)));
            }
        }
    }
}

TEST(ScribbleGestureRecognizer, preservesDecisionWhenDrawingIsReversedOrReflected) {
    Path points = makeScribble();
    std::reverse(points.begin(), points.end());
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
    for (Point& point: points) {
        point.x = -point.x;
    }
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, rejectsOrdinaryDrawings) {
    const auto drawings = ordinaryDrawings();
    for (size_t i = 0; i < drawings.size(); ++i) {
        SCOPED_TRACE(i);
        EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(drawings[i]));
    }
}

TEST(ScribbleGestureRecognizer, rejectsEmptyStationaryAndTinyTrajectories) {
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate({}));
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(Path{{3, 4}}));
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(Path(100, Point(3, 4))));
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(transform(makeScribble(), 0.0, 1e-6, 0, 0)));
}

TEST(ScribbleGestureRecognizer, toleratesRepeatedConsecutivePoints) {
    Path repeated;
    for (const Point& point: makeScribble()) {
        repeated.insert(repeated.end(), 30, point);
    }
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(repeated));
    for (const Path& drawing: ordinaryDrawings()) {
        repeated.clear();
        for (const Point& point: drawing) {
            repeated.insert(repeated.end(), 10, point);
        }
        EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(repeated));
    }
}

TEST(ScribbleGestureRecognizer, toleratesIrregularEventDensity) {
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(subdivide(makeScribble())));
    for (const Path& drawing: ordinaryDrawings()) {
        EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(subdivide(drawing)));
    }
}

TEST(ScribbleGestureRecognizer, toleratesSmallPenJitter) {
    Path points = subdivide(makeScribble(6));
    for (size_t i = 1; i + 1 < points.size(); ++i) {
        points[i].x += 0.1 * std::sin(static_cast<double>(i));
        points[i].y += 0.2 * std::cos(static_cast<double>(i));
    }
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, doesNotCountLocalTremorAsFullPasses) {
    Path points{{0, 0}, {80, 0}};
    for (size_t i = 0; i < 80; ++i) {
        points.emplace_back(i % 2 == 0 ? 70.0 : 80.0, 0.01 * static_cast<double>(i));
    }
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(points));
    points.clear();
    for (size_t i = 0; i < 500; ++i) {
        points.emplace_back(0.2 * static_cast<double>(i), 0.1 * std::sin(static_cast<double>(i)));
    }
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, rejectsAmbiguousWideAndDriftingTrajectories) {
    const Path wide{{0, 0}, {80, 0}, {0, 70}, {80, 70}, {0, 0}, {80, 0}, {0, 70}};
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(wide));
    const Path drifting{{0, 0}, {80, 2}, {50, 4}, {130, 6}, {100, 8}, {180, 10}, {150, 12}};
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(drifting));
    Path outlier = makeScribble();
    outlier.emplace_back(1000, 200);
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(outlier));
}

TEST(ScribbleGestureRecognizer, rejectsTrajectoriesLostByResampling) {
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(makeScribble(512)));
}

TEST(ScribbleGestureRecognizer, rejectsShortOffAxisExcursions) {
    const Path points{{0, 0}, {38.67, 0}, {38.67, 22}, {38.67, 0}, {80, 0}, {0, 0}, {80, 0}, {0, 0}};
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, recognizesCurvedPasses) {
    Path points;
    for (size_t pass = 0; pass < 4; ++pass) {
        for (size_t i = 0; i <= 40; ++i) {
            const double t = static_cast<double>(i) / 40.0;
            points.emplace_back(pass % 2 == 0 ? 80.0 * t : 80.0 * (1.0 - t),
                                2.0 * (static_cast<double>(pass) + t) + 4.0 * std::sin(std::numbers::pi * t));
        }
    }
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, rejectsInvalidCoordinatesAtEveryPosition) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const double largest = std::numeric_limits<double>::max();
    for (double invalid: {nan, infinity, -infinity, largest, -largest}) {
        for (size_t i = 0; i < makeScribble().size(); ++i) {
            SCOPED_TRACE(::testing::Message() << invalid << ", " << i);
            Path points = makeScribble();
            points[i].x = invalid;
            EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(points));
            points = makeScribble();
            points[i].y = invalid;
            EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(points));
        }
    }
    Path overflowing = makeScribble();
    overflowing.front().x = -largest;
    overflowing.back().x = largest;
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(overflowing));
}

TEST(ScribbleGestureRecognizer, ignoresPressureAndDoesNotMutateInput) {
    Path points = makeScribble();
    for (size_t i = 0; i < points.size(); ++i) {
        points[i].z = static_cast<double>(i);
    }
    const Path original = points;
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
    EXPECT_EQ(points, original);
    points.front().z = std::numeric_limits<double>::quiet_NaN();
    points.back().z = std::numeric_limits<double>::infinity();
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, handlesLargeFiniteCoordinates) {
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(transform(makeScribble(), 0.7, 1.0, 1e12, -1e12)));
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(transform(makeScribble(), 0.7, 1e150, 1e152, -1e152)));
}

TEST(ScribbleGestureRecognizer, handlesLongDenselySampledInput) {
    Path points;
    for (size_t pass = 0; pass < 4; ++pass) {
        for (size_t i = 0; i < 25000; ++i) {
            const double t = static_cast<double>(i) / 24999.0;
            points.emplace_back(pass % 2 == 0 ? 80.0 * t : 80.0 * (1.0 - t), 2.0 * (static_cast<double>(pass) + t));
        }
    }
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(points));
}

TEST(ScribbleGestureRecognizer, acceptsPartialPenDownAndPenUpPassesAroundCompleteScrubs) {
    const std::vector<Path> paths{
            {{0, 0}, {80, 1}, {0, 2}, {80, 3}, {0, 4}, {40, 5}},
            {{40, 0}, {80, 1}, {0, 2}, {80, 3}, {0, 4}, {80, 5}},
            {{40, 0}, {80, 1}, {0, 2}, {80, 3}, {0, 4}, {80, 5}, {40, 6}},
    };
    for (const auto& path: paths) {
        EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(path));
    }
}

TEST(ScribbleGestureRecognizer, boundaryFragmentsDoNotReplaceRequiredPassesOrExcuseInteriorFragments) {
    const std::vector<Path> paths{
            {{40, 0}, {80, 1}, {0, 2}, {80, 3}, {0, 4}, {40, 5}},
            {{0, 0}, {80, 1}, {0, 2}, {20, 3}, {0, 4}, {80, 5}, {0, 6}, {80, 7}, {0, 8}},
    };
    for (const auto& path: paths) {
        EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(path));
    }
}

TEST(ScribbleGestureRecognizer, boundsDenseUnevenPassStorage) {
    for (size_t passes: {10U, 32U, 64U, 65U, 512U}) {
        SCOPED_TRACE(passes);
        Path path;
        for (size_t i = 0; i <= passes; ++i) {
            path.emplace_back(i % 2 ? 80.0 : 0.0, 20 * std::sin(static_cast<double>(i) * 0.7));
        }
        EXPECT_EQ(ScribbleGestureRecognizer::isCandidate(path), passes <= 64);
    }
}

TEST(ScribbleGestureRecognizer, configuredPassCountChangesRecognition) {
    for (int minimum: {2, 3, 4, 6, 8}) {
        for (int passes = 1; passes <= 8; ++passes) {
            EXPECT_EQ(ScribbleGestureRecognizer::isCandidate(makeScribble(static_cast<size_t>(passes)), {50, minimum}),
                      passes >= minimum)
                    << "minimum=" << minimum << " actual=" << passes;
        }
    }
}

TEST(ScribbleGestureRecognizer, sensitivityChangesToleranceForMovingScrubs) {
    Path path;
    for (size_t i = 0; i <= 12; ++i) {
        path.emplace_back(10.0 * static_cast<double>(i), i % 2 ? 40.0 : 0.0);
    }
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(path, {0, 4}));
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(path, {50, 4}));
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate(path, {100, 4}));
    EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate({path.data(), 7}, {50, 4}));
    EXPECT_TRUE(ScribbleGestureRecognizer::isCandidate({path.data(), 7}, {100, 4}));
}

TEST(ScribbleGestureRecognizer, clampsExternalOptionsAndPreservesSingleCrossings) {
    for (ScribbleSettings settings:
         {ScribbleSettings{-100, -100}, ScribbleSettings{500, 500}, ScribbleSettings{100, 2}}) {
        const Path line{{0, 0}, {80, 0}};
        EXPECT_FALSE(ScribbleGestureRecognizer::isCandidate(line, settings));
        EXPECT_EQ(ScribbleGestureRecognizer::isCandidate(makeScribble(8), settings),
                  ScribbleGestureRecognizer::isCandidate(makeScribble(8), settings.normalized()));
    }
}
