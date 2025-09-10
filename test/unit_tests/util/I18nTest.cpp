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

#include "util/i18n.h"

#include "config-test.h"


using namespace std;


TEST(UtilI18n, testNoPlaceholder) {
    string msg = FS(FORMAT_STR("test123") % 1 % 2 % 3);

    EXPECT_EQ(std::string("test123"), msg);
}

TEST(UtilI18n, testEscape) {
    string msg = FS(FORMAT_STR("test{{123") % 1 % 2 % 3);

    EXPECT_EQ(std::string("test{123"), msg);
}

TEST(UtilI18n, testReplace) {
    string msg = FS(FORMAT_STR("aa {1} bb {1} {2}") % 1 % 2 % 3);

    EXPECT_EQ(std::string("aa 1 bb 1 2"), msg);
}

TEST(UtilI18n, testMissing) {
    string msg = FS(FORMAT_STR("aa {1} bb {1} {2}"));

    EXPECT_EQ(std::string("aa {1} bb {1} {2}"), msg);
}

TEST(UtilI18n, testOrder) {
    string msg = FS(FORMAT_STR(".. {2} .. {1} -- {2} {1}") % "a" % "b");

    EXPECT_EQ(std::string(".. b .. a -- b a"), msg);
}

TEST(UtilI18n, testLatexString) {
    string command =
            FS(FORMAT_STR("{1} -m 0 \"\\png\\usepackage{{color}}\\color{{{2}}}\\dpi{{{3}}}\\normalsize {4}\" -o {5}") %
               "abc" % "red" % 45 % "asdf" % "asdf.png");
    EXPECT_EQ(std::string("abc -m 0 \"\\png\\usepackage{color}\\color{red}\\dpi{45}\\normalsize asdf\" -o asdf.png"),
              command);
}

TEST(UtilI18n, test3) {
    string msg = FS(FORMAT_STR(" of {1}{2}") % 5 % 6);
    EXPECT_EQ(std::string(" of 56"), msg);
}

TEST(UtilI18n, test16bit) {
    string msg = FS(FORMAT_STR("{1} = {2} and {3}") % 60123 % 60123U % -65536);
    EXPECT_EQ(std::string("60123 = 60123 and -65536"), msg);
}

TEST(UtilI18n, test32bit) {
    string msg = FS(FORMAT_STR("{1} and {2}") % 4294967295U % -12345678);
    EXPECT_EQ(std::string("4294967295 and -12345678"), msg);
}

TEST(UtilI18n, test64bit) {
    string msg = FS(FORMAT_STR("{1} and {2}") % 1234567890123456789U % -1234567890123456789);
    EXPECT_EQ(std::string("1234567890123456789 and -1234567890123456789"), msg);
}
