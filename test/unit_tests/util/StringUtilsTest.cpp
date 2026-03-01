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

#include <cstdlib>
#include <ctime>

#include <glib.h>
#include <gtest/gtest.h>

#include "util/StringUtils.h"

#include "config-test.h"


using namespace std;


TEST(UtilStringUtils, testStartWith) {
    EXPECT_EQ(true, StringUtils::startsWith("asdfsfdafdasfda", "asdf"));
    EXPECT_EQ(false, StringUtils::startsWith("111111111111111", "2222"));
    EXPECT_EQ(false, StringUtils::startsWith("122221111111111", "2222"));
    EXPECT_EQ(false, StringUtils::startsWith("", "asdf"));
    EXPECT_EQ(true, StringUtils::startsWith("aaaaaaa", ""));
}

TEST(UtilStringUtils, testSplit) {
    vector<string> splitted = StringUtils::split("a,,b,c,d,e,f", ',');

    EXPECT_EQ(7, (int)splitted.size());
    EXPECT_EQ(std::string("a"), splitted[0]);
    EXPECT_EQ(std::string(""), splitted[1]);
}

TEST(UtilStringUtils, testSplitEmpty) {
    vector<string> splitted = StringUtils::split("", ',');

    EXPECT_EQ(0, (int)splitted.size());
}

TEST(UtilStringUtils, testSplitOne) {
    vector<string> splitted = StringUtils::split("aa", ',');

    EXPECT_EQ(1, (int)splitted.size());
    EXPECT_EQ(std::string("aa"), splitted[0]);
}

TEST(UtilStringUtils, testEndsWith) {
    EXPECT_EQ(true, StringUtils::endsWith("asdfsfdafdasfda.xoj", ".xoj"));
    EXPECT_EQ(false, StringUtils::endsWith("111111111111111", "2222"));
    EXPECT_EQ(false, StringUtils::endsWith("111111111122221", "2222"));
    EXPECT_EQ(false, StringUtils::endsWith("", "asdf"));
    EXPECT_EQ(true, StringUtils::endsWith("aaaaaaa", ""));
}

TEST(UtilStringUtils, testCompare) {
    EXPECT_EQ(true, StringUtils::iequals("", ""));
    EXPECT_EQ(true, StringUtils::iequals("aaaaaaaa", "aAAAaaaa"));
    EXPECT_EQ(true, StringUtils::iequals("äää", "ÄÄÄ"));
    EXPECT_EQ(true, StringUtils::iequals("ööaa", "Ööaa"));
    EXPECT_EQ(false, StringUtils::iequals("ööaa", "ööaaa"));
}

TEST(UtilStringUtils, testEllipsize) {
    struct TestCase {
        std::string_view str;   // original string
        std::size_t max_width;  // in code points
        bool ellipsized;        // expected result
    };

    constexpr std::array<TestCase, 5> cases = {{
            {"short string", 20, false},
            {"very looooooooong string", 20, true},
            {"éééééaouüüüüüüöööödèèû", 10, true},
            {"CJK \xE4\xB8\x96\xE7\x95\x8C", 6, false},
            {"\xE4\xB8\x96\xE7\x95\x8C\xE4\xB8\x96\xE7\x95\x8C\xE4\xB8\x96\xE7\x95\x8C", 5, true},
    }};
    for (const auto& tc: cases) {
        auto out = StringUtils::ellipsize(tc.str, tc.max_width);

        EXPECT_TRUE(g_utf8_validate(out.c_str(), out.size(), nullptr))
                << "Ellipsizing string \"" << tc.str << "\" produced invalid UTF-8";

        if (tc.ellipsized) {
            EXPECT_EQ(g_utf8_strlen(out.c_str(), out.size()), tc.max_width)
                    << "Ellipsizing string \"" << tc.str << "\" produced a string of the wrong size";
            EXPECT_TRUE(StringUtils::endsWith(out, "...")) << '"' << out << "\" does not end with an ellipsis";
        } else {
            EXPECT_EQ(out, tc.str) << "String should not have been ellipsized";
        }
    }
}
