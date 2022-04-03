#include <array>
#include <map>

#include <gtest/gtest.h>

#include "util/UnionOfIntervals.h"

TEST(UtilIntervals, testInterval) {
    std::array<Interval<double>, 4> intervals = {Interval<double>::getInterval(12.34, 1),
                                                 Interval<double>::getInterval(0, 2), Interval<double>(3, 4),
                                                 Interval<double>(-0.2, 3)};


    // Test intersection
    std::map<std::pair<size_t, size_t>, std::optional<Interval<double>>> intersections = {
            {{0, 1}, Interval<double>(1, 2)}, {{0, 2}, Interval<double>(3, 4)}, {{0, 3}, Interval<double>(1, 3)},
            {{1, 2}, std::nullopt},           {{1, 3}, Interval<double>(0, 2)}, {{2, 3}, std::nullopt}};

    for (size_t i = 0; i < intervals.size() - 1; ++i) {
        for (size_t j = i + 1; j < intervals.size(); ++j) {
            auto intersection = intersections[{i, j}];
            auto intersectionAB = intervals[i].intersect(intervals[j]);
            EXPECT_EQ(intersectionAB.has_value(), intersection.has_value());

            auto intersectionBA = intervals[j].intersect(intervals[i]);
            EXPECT_EQ(intersectionBA.has_value(), intersection.has_value());

            if (intersection.has_value()) {
                EXPECT_EQ(intersectionBA.value().min, intersection.value().min);
                EXPECT_EQ(intersectionBA.value().max, intersection.value().max);
                EXPECT_EQ(intersectionAB.value().min, intersection.value().min);
                EXPECT_EQ(intersectionAB.value().max, intersection.value().max);
            }
        }
    }

    // Test isContainedIn
    std::array<std::array<bool, 4>, 4> isContainedIn = {
            std::array<bool, 4>{true, false, false, false}, std::array<bool, 4>{false, true, false, true},
            std::array<bool, 4>{true, false, true, false}, std::array<bool, 4>{false, false, false, true}};
    for (size_t i = 0; i < intervals.size(); ++i) {
        for (size_t j = 0; j < intervals.size(); ++j) {
            EXPECT_EQ(isContainedIn[i][j], intervals[i].isContainedIn(intervals[j]));
        }
    }

    // Test IntervalIteratable
    std::vector<Interval<double>> expectedResults = {{1, 12.34}, {0, 2}, {3, 4}, {-0.2, 3}};
    size_t i = 0;
    for (auto interval: expectedResults) {
        EXPECT_EQ(interval.min, intervals[i].min);
        EXPECT_EQ(interval.max, intervals[i].max);
        ++i;
    }

    // Test envelop
    std::array<Interval<double>, 4> envelop2point5 = {Interval<double>{1, 12.34}, Interval<double>{0, 2.5},
                                                      Interval<double>{2.5, 4}, Interval<double>{-0.2, 3}};
    for (size_t i = 0; i < intervals.size(); ++i) {
        intervals[i].envelop(2.5);
        EXPECT_EQ(intervals[i].min, envelop2point5[i].min);
        EXPECT_EQ(intervals[i].max, envelop2point5[i].max);
    }
}

TEST(UtilIntervals, testUnionOfIntervals) {
    std::vector<double> bounds = {0, 1, 3, 5, 7, 10};
    UnionOfIntervals<double> intervals;
    intervals.appendData(bounds);

    {  // Test clone
        auto clone = intervals.cloneToIntervalVector();
        EXPECT_TRUE(bounds.size() % 2 == 0);
        EXPECT_EQ(clone.size(), bounds.size() / 2);
        auto it = bounds.begin();
        for (auto& interval: clone) {
            EXPECT_EQ(interval.min, *it);
            ++it;
            EXPECT_EQ(interval.max, *it);
            ++it;
        }
    }

    {  // Test unite
        UnionOfIntervals<double> intervals2;
        std::vector<double> bounds2 = {-2, -1, -0.5, 0.5, 2.5, 3.5, 4, 8};
        intervals2.appendData(bounds2);
        intervals2.unite(intervals.getData());
        auto unionOfTheTwo = intervals2.cloneToIntervalVector();
        std::vector<Interval<double>> expectedRes = {{-2, -1}, {-0.5, 1}, {2.5, 10}};
        EXPECT_EQ(expectedRes.size(), unionOfTheTwo.size());
        auto it = unionOfTheTwo.begin();
        for (auto& interval: expectedRes) {
            EXPECT_EQ(interval.min, it->min);
            EXPECT_EQ(interval.max, it->max);
            ++it;
        }
    }

    {  // Test intersect
        UnionOfIntervals<double> intervals2;
        std::vector<double> bounds2 = {-2, -1, -0.5, 0.5, 2.5, 3.5, 4, 8};
        intervals2.appendData(bounds2);
        intervals2.intersect(intervals.getData());
        auto interOfTheTwo = intervals2.cloneToIntervalVector();
        std::vector<Interval<double>> expectedRes = {{0, 0.5}, {3, 3.5}, {4, 5}, {7, 8}};
        EXPECT_EQ(expectedRes.size(), interOfTheTwo.size());
        auto it = interOfTheTwo.begin();
        for (auto& interval: expectedRes) {
            EXPECT_EQ(interval.min, it->min);
            EXPECT_EQ(interval.max, it->max);
            ++it;
        }
    }
}
