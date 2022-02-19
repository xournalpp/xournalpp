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

#ifdef _WIN32

/*
 * Reference samples obtained with the following python functions:
 *      def printUTF8(s: str):
 *          b = bytes(s, "UTF-8")
 *          print("".join("\\x{0:02x}".format(p) for p in b))
 *
 *      def printUTF16(s: str):
 *          b = bytes(s, "UTF-16-BE")
 *          print("".join("\\x{0:02x}{1:02x}".format(*p) for p in zip(b[0::2], b[1::2])))
 */

TEST(UtilStringCvt, u16u8Conv1) {
    // Ends with ASCII character
    const string u8sample{u8"The fox 🦊 jumps 🏃‍♂️ over the dog's 🐕‍🦺 bones 🦴🍖."};
    const wstring u16sample{L"The fox 🦊 jumps 🏃‍♂️ over the dog's 🐕‍🦺 bones 🦴🍖."};

    const string u8ref{
            "\x54\x68\x65\x20\x66\x6f\x78\x20\xf0\x9f\xa6\x8a\x20\x6a\x75\x6d\x70\x73\x20\xf0\x9f\x8f\x83\xe2\x80\x8d"
            "\xe2\x99\x82\xef\xb8\x8f\x20\x6f\x76\x65\x72\x20\x74\x68\x65\x20\x64\x6f\x67\x27\x73\x20\xf0\x9f\x90\x95"
            "\xe2\x80\x8d\xf0\x9f\xa6\xba\x20\x62\x6f\x6e\x65\x73\x20\xf0\x9f\xa6\xb4\xf0\x9f\x8d\x96\x2e"};
    const wstring u16ref{L"\x0054\x0068\x0065\x0020\x0066\x006f\x0078\x0020\xd83e\xdd8a\x0020\x006a\x0075\x006d\x0070"
                         L"\x0073\x0020\xd83c\xdfc3\x200d\x2642\xfe0f\x0020\x006f\x0076\x0065\x0072\x0020\x0074\x0068"
                         L"\x0065\x0020\x0064\x006f\x0067\x0027\x0073\x0020\xd83d\xdc15\x200d\xd83e\xddba\x0020\x0062"
                         L"\x006f\x006e\x0065\x0073\x0020\xd83e\xddb4\xd83c\xdf56\x002e"};

    assert(u8sample == u8ref);
    assert(u16sample == u16ref);

    EXPECT_EQ(StringCvt::u16(u8sample), u16sample);
    EXPECT_EQ(StringCvt::u8(u16sample), u8sample);
}

TEST(UtilStringCvt, u16u8Conv2) {
    // Ends with 'upper BMP' character (multiple UTF-8 code units but single UTF-16 code unit)
    const string u8sample{u8"🌍🌎🌏🛰🚀⏳✌"};
    const wstring u16sample{L"🌍🌎🌏🛰🚀⏳✌"};

    const string u8ref{
            "\xf0\x9f\x8c\x8d\xf0\x9f\x8c\x8e\xf0\x9f\x8c\x8f\xf0\x9f\x9b\xb0\xf0\x9f\x9a\x80\xe2\x8f\xb3\xe2\x9c\x8c"};
    const wstring u16ref{L"\xd83c\xdf0d\xd83c\xdf0e\xd83c\xdf0f\xd83d\xdef0\xd83d\xde80\x23f3\x270c"};

    assert(u8sample == u8ref);
    assert(u16sample == u16ref);

    EXPECT_EQ(StringCvt::u16(u8sample), u16sample);
    EXPECT_EQ(StringCvt::u8(u16sample), u8sample);
}

TEST(UtilStringCvt, u16u8Conv3) {
    // Ends with non-BMP character (multiple UTF-8 and UTF-16 code units)
    const string u8sample{u8"Going 🚙🌴🌵"};
    const wstring u16sample{L"Going 🚙🌴🌵"};

    const string u8ref{"\x47\x6f\x69\x6e\x67\x20\xf0\x9f\x9a\x99\xf0\x9f\x8c\xb4\xf0\x9f\x8c\xb5"};
    const wstring u16ref{L"\x0047\x006f\x0069\x006e\x0067\x0020\xd83d\xde99\xd83c\xdf34\xd83c\xdf35"};

    assert(u8sample == u8ref);
    assert(u16sample == u16ref);

    EXPECT_EQ(StringCvt::u16(u8sample), u16sample);
    EXPECT_EQ(StringCvt::u8(u16sample), u8sample);
}

TEST(UtilStringCvt, u16u8ConvEmpty) {
    const string u8sample{u8""};
    const wstring u16sample{L""};

    EXPECT_EQ(StringCvt::u16(u8sample), u16sample);
    EXPECT_EQ(StringCvt::u8(u16sample), u8sample);
}

TEST(UtilStringCvt, u16u8ConvFail) {
    const string u8sample{"asdf\xc3\xb6l\xc1r"};             // invalid UTF-8 sequence at index 7
    const wstring u16sample{L"asdf\xd83d\xdeb4gl\xd985go"};  // invalid UTF-16 sequence at index 8

    EXPECT_THROW(
            try { StringCvt::u16(u8sample); } catch (StringCvt::ConversionError& e) {
                EXPECT_EQ(e.index(), 7);
                EXPECT_EQ(e.result(), std::codecvt_base::error);
                throw;
            },
            StringCvt::ConversionError);

    EXPECT_THROW(
            try { StringCvt::u8(u16sample); } catch (StringCvt::ConversionError& e) {
                EXPECT_EQ(e.index(), 8);
                EXPECT_EQ(e.result(), std::codecvt_base::error);
                throw;
            },
            StringCvt::ConversionError);
}

#endif  // _WIN32
