#include <array>

#include <gtest/gtest.h>

#include "util/Range.h"

namespace {
struct TestData {
    TestData(const Range& rg, bool isEmpty, bool isValid): rg(rg), isEmpty(isEmpty), isValid(isValid) {}
    Range rg;
    bool isEmpty;
    bool isValid;
};

std::array<TestData, 5> testData = {
        //
        TestData(Range(), true, false),                      //
        TestData(Range(1.0, 0.0), false, true),              //
        TestData(Range(0.0, 0.0, 1.0, 1.0), false, true),    //
        TestData(Range(0.0, 0.0, -1.0, 1.0), false, false),  //
        TestData(Range(0.0, 0.0, 1.0, 0.0), false, true)     //
};

bool equal(const Range& r1, const Range& r2) {
    return r1.minX == r2.minX && r1.minY == r2.minY && r1.maxX == r2.maxX && r1.maxY == r2.maxY;
}
};  // namespace

TEST(UtilRange, testEmptyValid) {
    for (auto& d: testData) {
        EXPECT_EQ(d.rg.empty(), d.isEmpty);
        EXPECT_EQ(d.rg.isValid(), d.isValid);
    }
}

TEST(UtilRange, testUnite) {
    Range emptyRange;
    for (auto& d: testData) {
        EXPECT_TRUE(equal(d.rg.unite(emptyRange), d.rg));
    }

    // intersecting ranges
    EXPECT_TRUE(equal(Range(0.0, -1.0, 1.0, 2.0).unite(Range(0.5, 1.0, 0.6, 3.0)), Range(0.0, -1.0, 1.0, 3.0)));
    // subrange
    EXPECT_TRUE(equal(Range(0.0, -1.0, 1.0, 2.0).unite(Range(0.5, 1.0, 0.6, 1.5)), Range(0.0, -1.0, 1.0, 2.0)));
    // disjoint ranges
    EXPECT_TRUE(equal(Range(0.0, -1.0, 1.0, 2.0).unite(Range(1.5, 3.0, 2.0, 4.0)), Range(0.0, -1.0, 2.0, 4.0)));
}

TEST(UtilRange, testIntersect) {
    Range emptyRange;
    for (auto& d: testData) {
        EXPECT_TRUE(d.rg.intersect(emptyRange).empty());
    }

    // intersecting ranges
    EXPECT_TRUE(equal(Range(0.0, -1.0, 1.0, 2.0).intersect(Range(0.5, 1.0, 0.6, 3.0)), Range(0.5, 1.0, 0.6, 2.0)));
    // subrange
    EXPECT_TRUE(equal(Range(0.0, -1.0, 1.0, 2.0).intersect(Range(0.5, 1.0, 0.6, 1.5)), Range(0.5, 1.0, 0.6, 1.5)));
    // disjoint ranges
    EXPECT_TRUE(Range(0.0, -1.0, 1.0, 2.0).intersect(Range(1.5, 3.0, 2.0, 4.0)).empty());
}
