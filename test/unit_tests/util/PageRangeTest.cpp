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


#include <ostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "util/PageRange.h"


bool operator==(const PageRangeEntry& lhs, const PageRangeEntry& rhs) {
    return lhs.first == rhs.first && lhs.last == rhs.last;
}

TEST(UtilPageRange, testParse) {
    std::string ranges = "1, 2-, -3, 4-5, -";
    PageRangeVector result{{0, 0}, {1, 9}, {0, 2}, {3, 4}, {0, 9}};

    auto v_10 = PageRange::parse(ranges, 10);

    ASSERT_EQ(v_10.size(), result.size());

    auto it1 = v_10.cbegin();
    auto it2 = result.cbegin();
    while (it1 != v_10.cend()) { ASSERT_EQ(*it1++, *it2++); }
}

TEST(UtilPageRange, testInvalid) {
    const std::vector<std::string> bad_inputs = {
            "github", "",   "  \t\t   \n\r\n  \r\r           ", "1-3, 6; HELLOWORLD ; 4-  : -5,, 6-5, 99-, -", "5-11",
            "0-3",    "6-5"};
    size_t pageCount = 10;
    for (auto& bad_input: bad_inputs) {
        try {
            auto actual = PageRange::parse(bad_input, pageCount);
            FAIL() << "std::invalid_argument not thrown for bad input.";
        } catch (const std::invalid_argument& e) {
            // good, exception is thrown as it should
        } catch (const std::exception& e) { FAIL() << e.what(); } catch (...) {
            FAIL() << "Unexpected exception caught.";
        }
    }
}

TEST(UtilPageRange, testPageCountIsZero) {
    try {
        auto actual = PageRange::parse("", 0);
        FAIL() << "std::logic_error not thrown when pageCount equals 0.";
    } catch (const std::logic_error& e) {
        // good, exception is thrown as it should
    } catch (const std::exception& e) { FAIL() << e.what(); } catch (...) {
        FAIL() << "Unexpected exception caught.";
    }
}
