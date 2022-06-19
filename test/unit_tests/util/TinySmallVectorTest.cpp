#include <array>
#include <map>

#include <gtest/gtest.h>

#include "util/SmallVector.h"
#include "util/TinyVector.h"

namespace {
class TestData {
public:
    TestData() { nbData++; };
    ~TestData() { nbData--; };
    TestData(double t, bool duplicate): t(t) {
        if (duplicate) {
            u = t;
        }
        nbData++;
    }
    TestData(double t, double u): t(t), u(u) { nbData++; }
    TestData(const TestData& other): t(other.t), u(other.u) { nbData++; }
    TestData(TestData&& other): t(other.t), u(other.u) { nbData++; }
    TestData& operator=(const TestData& other) {
        t = other.t;
        u = other.u;
        return *this;
    }
    TestData& operator=(TestData&& other) {
        t = other.t;
        u = other.u;
        return *this;
    }
    double t = 0.0;
    double u = 0.0;
    bool operator==(const TestData& other) const { return t == other.t && u == other.u; }

    static int nbData;
};
int TestData::nbData = 0;
};  // namespace

TEST(UtilsVectors, testTinyVector) {
    TestData::nbData = 0;

    {  // scope for vec. To test destructor after.
        TinyVector<TestData, 5> vec;
        EXPECT_EQ(vec.size(), 0);
        EXPECT_TRUE(vec.empty());

        {  // d1 scope, for a correct TestData::nbData
            TestData d1(1.0, 2.0);

            vec.push_back(d1);
            EXPECT_EQ(vec.size(), 1);
            vec.push_back(std::move(d1));
            EXPECT_EQ(vec.size(), 2);
            vec.emplace_back(d1);
            EXPECT_EQ(vec.size(), 3);
            vec.emplace_back(3.0, true);
            EXPECT_EQ(vec.size(), 4);
            vec.emplace_back(4.0, false);
            EXPECT_EQ(vec.size(), 5);
        }

        EXPECT_EQ(TestData::nbData, 5);

        EXPECT_EQ(vec[0], vec.front());
        EXPECT_EQ(vec[0], TestData(1.0, 2.0));
        EXPECT_EQ(vec[1], TestData(1.0, 2.0));
        EXPECT_EQ(vec[2], TestData(1.0, 2.0));
        EXPECT_EQ(vec[3], TestData(3.0, 3.0));
        EXPECT_EQ(vec[4], TestData(4.0, 0.0));
        EXPECT_EQ(vec.back(), vec[4]);

        //     try {
        //         vec.push_back(d1);
        //         FAIL() << "TinyVector should have thrown std::length_error";
        //     } catch (std::length_error& e) {}
        //
        //     try {
        //         vec.push_back(std::move(d1));
        //         FAIL() << "TinyVector should have thrown std::length_error";
        //     } catch (std::length_error& e) {}
        //
        //     try {
        //         vec.emplace_back(d1);
        //         FAIL() << "TinyVector should have thrown std::length_error";
        //     } catch (std::length_error& e) {}

        vec.pop_back();
        EXPECT_EQ(vec.size(), 4);
        EXPECT_EQ(TestData::nbData, 4);

        // Test range loop
        for (auto& d: vec) { d.u += d.t; }
        EXPECT_EQ(vec[0], TestData(1.0, 3.0));
        EXPECT_EQ(vec[1], TestData(1.0, 3.0));
        EXPECT_EQ(vec[2], TestData(1.0, 3.0));
        EXPECT_EQ(vec[3], TestData(3.0, 6.0));

        // Test clear
        vec.clear();
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(vec.size(), 0);
        EXPECT_EQ(TestData::nbData, 0);

        vec.emplace_back(3.0, true);
        vec.emplace_back(4.0, false);
        vec.emplace_back(5.0, true);
        vec.emplace_back(6.0, false);

        EXPECT_EQ(vec.size(), 4);
        EXPECT_EQ(TestData::nbData, 4);

        {  // Test copy/move assignment
            auto vec2 = vec;
            EXPECT_EQ(vec2.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
            EXPECT_EQ(TestData::nbData, 8);

            auto vec3 = std::move(vec2);
            EXPECT_TRUE(vec2.empty());
            EXPECT_EQ(vec3.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
            EXPECT_EQ(TestData::nbData, 8);

            // Self assignment
            auto* vec4 = &vec3;
            vec3 = *vec4;
            EXPECT_EQ(vec3.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
            EXPECT_EQ(TestData::nbData, 8);
            vec3 = std::move(*vec4);
            EXPECT_EQ(vec3.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
            EXPECT_EQ(TestData::nbData, 8);

            // Swap
            vec2.emplace_back(1.0, 2.0);
            std::swap(vec2, vec3);
            EXPECT_EQ(vec2.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
            EXPECT_EQ(vec3.size(), 1);
            EXPECT_EQ(TestData::nbData, 9);
        }
        EXPECT_EQ(TestData::nbData, 4);

        {  // Test copy/move constructor
            auto vec2(vec);
            EXPECT_EQ(vec2.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
            EXPECT_EQ(TestData::nbData, 8);

            auto vec3(std::move(vec2));
            EXPECT_TRUE(vec2.empty());
            EXPECT_EQ(vec3.size(), 4);
            EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
            EXPECT_EQ(TestData::nbData, 8);
        }
        EXPECT_EQ(TestData::nbData, 4);
    }  // end of scope for vec
    EXPECT_EQ(TestData::nbData, 0);
}

TEST(UtilsVectors, testSmallVectorPushPop) {
    TestData::nbData = 0;

    SmallVector<TestData, 3> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());

    TestData d1(1.0, 2.0);
    TestData d2(-1.0, -2.0);
    TestData d3(-1.0, 2.0);

    EXPECT_EQ(TestData::nbData, 0 + 3);  // 3 for d1, d2 and d3

    vec.push_back(d1);
    EXPECT_EQ(vec.size(), 1);
    vec.push_back(std::move(d2));
    EXPECT_EQ(vec.size(), 2);
    vec.emplace_back(d3);
    EXPECT_EQ(vec.size(), 3);

    EXPECT_EQ(TestData::nbData, 3 + 3);  // 3 for d1, d2 and d3

    vec.emplace_back(3.0, true);  // Move from stack to heap
    EXPECT_EQ(vec.size(), 4);
    vec.emplace_back(4.0, false);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(TestData::nbData, 5 + 3);  // 3 for d1, d2 and d3

    EXPECT_EQ(vec[0], vec.front());
    EXPECT_EQ(vec[0], d1);
    EXPECT_EQ(vec[1], d2);
    EXPECT_EQ(vec[2], d3);
    EXPECT_EQ(vec[3], TestData(3.0, 3.0));
    EXPECT_EQ(vec[4], TestData(4.0, 0.0));
    EXPECT_EQ(vec.back(), vec[4]);

    vec.pop_back();
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(TestData::nbData, 4 + 3);
    vec.pop_back();  // Move from heap to stack
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(TestData::nbData, 3 + 3);  // 3 for d1, d2 and d3
    vec.pop_back();
    EXPECT_EQ(vec.size(), 2);

    EXPECT_EQ(vec.front(), d1);
    EXPECT_EQ(vec.back(), d2);

    // Loop in stack
    for (auto& d: vec) { d.t += d.u; }
    EXPECT_EQ(vec.front(), TestData(3.0, 2.0));
    EXPECT_EQ(vec.back(), TestData(-3.0, -2.0));
    EXPECT_EQ(TestData::nbData, 3 + 2);

    vec.emplace_back(d3);
    EXPECT_EQ(vec.size(), 3);
    vec.emplace_back(3.0, true);
    EXPECT_EQ(vec.size(), 4);
    vec.emplace_back(4.0, false);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(TestData::nbData, 5 + 3);  // 3 for d1, d2 and d3

    // Loop in heap
    for (auto& d: vec) { d.u += d.t; }
    EXPECT_EQ(vec[0], TestData(3.0, 5.0));
    EXPECT_EQ(vec[1], TestData(-3.0, -5.0));
    EXPECT_EQ(vec[2], TestData(-1.0, 1.0));
    EXPECT_EQ(vec[3], TestData(3.0, 6.0));
    EXPECT_EQ(vec[4], TestData(4.0, 4.0));

    vec.clear();
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(TestData::nbData, 0 + 3);  // 3 for d1, d2 and d3
}

TEST(UtilsVectors, testSmallVectorCopyMove) {
    TestData::nbData = 0;
    {  // copy and move assignments in stack
        SmallVector<TestData, 3> vec = {TestData(3.0, 1.0), TestData(-3.0, -3.0)};
        EXPECT_EQ(vec.size(), 2);
        EXPECT_EQ(TestData::nbData, 2);

        auto vec2 = vec;
        EXPECT_EQ(vec2.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);

        auto vec3 = std::move(vec2);
        EXPECT_EQ(vec2.size(), 0);
        EXPECT_EQ(vec3.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);

        // Self assignment
        auto* vec4 = &vec3;
        vec3 = *vec4;
        EXPECT_EQ(vec3.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);
        vec3 = std::move(*vec4);
        EXPECT_EQ(vec3.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);

        // Swap
        vec2.emplace_back(1.0, true);
        std::swap(vec2, vec3);
        EXPECT_EQ(vec2.size(), 2);
        EXPECT_EQ(vec3.size(), 1);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2 + 1);
    }

    {  // copy and move constructor in stack
        SmallVector<TestData, 3> vec{TestData(3.0, 1.0), TestData(-3.0, -3.0)};
        EXPECT_EQ(vec.size(), 2);
        EXPECT_EQ(TestData::nbData, 2);

        SmallVector<TestData, 3> vec2(vec);
        EXPECT_EQ(vec2.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);

        SmallVector<TestData, 3> vec3(std::move(vec2));
        EXPECT_EQ(vec2.size(), 0);
        EXPECT_EQ(vec3.size(), 2);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 2 + 2);
    }

    {  // assignment in heap
        SmallVector<TestData, 3> vec = {TestData(1.0, 2.0), TestData(1.0, 3.0), TestData(2.0, 2.0), TestData(4.0, 2.0),
                                        TestData(-1.0, 2.0)};
        EXPECT_EQ(TestData::nbData, 5);

        auto vec2 = vec;
        EXPECT_EQ(vec2.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);

        auto vec3 = std::move(vec2);
        EXPECT_EQ(vec2.size(), 0);
        EXPECT_EQ(vec3.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);

        // Self assignment
        auto* vec4 = &vec3;
        vec3 = *vec4;
        EXPECT_EQ(vec3.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);
        vec3 = std::move(*vec4);
        EXPECT_EQ(vec3.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);

        // Swap
        vec2.emplace_back(-1.0, true);
        std::swap(vec2, vec3);  // stack to heap
        EXPECT_EQ(vec2.size(), 5);
        EXPECT_EQ(vec3.size(), 1);
        EXPECT_EQ(vec3.front(), TestData(-1.0, true));
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5 + 1);
        std::swap(vec2, vec3);  // heap to stack
        EXPECT_EQ(vec2.size(), 1);
        EXPECT_EQ(vec3.size(), 5);
        EXPECT_EQ(vec2.front(), TestData(-1.0, true));
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5 + 1);

        vec2.emplace_back(2.0, 3.0);
        vec2.emplace_back(3.0, -3.0);
        vec2.emplace_back(4.0, 5.0);
        EXPECT_EQ(vec2.size(), 4);
        EXPECT_EQ(TestData::nbData, 5 + 5 + 4);
        std::swap(vec2, vec3);  // heap to heap
        EXPECT_EQ(vec2.size(), 5);
        EXPECT_EQ(vec3.size(), 4);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5 + 4);
    }

    {  // copy and move constructor in heap
        SmallVector<TestData, 3> vec{TestData(1.0, 2.0), TestData(1.0, 3.0), TestData(2.0, 2.0), TestData(4.0, 2.0),
                                     TestData(-1.0, 2.0)};
        EXPECT_EQ(TestData::nbData, 5);

        SmallVector<TestData, 3> vec2(vec);
        EXPECT_EQ(vec2.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec2.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);

        SmallVector<TestData, 3> vec3(std::move(vec2));
        EXPECT_EQ(vec2.size(), 0);
        EXPECT_EQ(vec3.size(), 5);
        EXPECT_TRUE(std::equal(vec.begin(), vec.end(), vec3.begin()));
        EXPECT_EQ(TestData::nbData, 5 + 5);
    }
    EXPECT_EQ(TestData::nbData, 0);
}
