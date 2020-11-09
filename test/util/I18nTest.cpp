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

#include <config-test.h>
#include <cppunit/extensions/HelperMacros.h>
#include <i18n.h>

using namespace std;

class I18nTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(I18nTest);

    CPPUNIT_TEST(testNoPlaceholder);
    CPPUNIT_TEST(testEscape);
    CPPUNIT_TEST(testReplace);
    CPPUNIT_TEST(testMissing);
    CPPUNIT_TEST(testOrder);
    CPPUNIT_TEST(testLatexString);
    CPPUNIT_TEST(test3);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

    void testNoPlaceholder() {
        string msg = FS(FORMAT_STR("test123") % 1 % 2 % 3);

        CPPUNIT_ASSERT_EQUAL(std::string("test123"), msg);
    }

    void testEscape() {
        string msg = FS(FORMAT_STR("test{{123") % 1 % 2 % 3);

        CPPUNIT_ASSERT_EQUAL(std::string("test{123"), msg);
    }

    void testReplace() {
        string msg = FS(FORMAT_STR("aa {1} bb {1} {2}") % 1 % 2 % 3);

        CPPUNIT_ASSERT_EQUAL(std::string("aa 1 bb 1 2"), msg);
    }

    void testMissing() {
        string msg = FS(FORMAT_STR("aa {1} bb {1} {2}"));

        CPPUNIT_ASSERT_EQUAL(std::string("aa {1} bb {1} {2}"), msg);
    }

    void testOrder() {
        string msg = FS(FORMAT_STR(".. {2} .. {1} -- {2} {1}") % "a" % "b");

        CPPUNIT_ASSERT_EQUAL(std::string(".. b .. a -- b a"), msg);
    }

    void testLatexString() {
        string command = FS(
                FORMAT_STR("{1} -m 0 \"\\png\\usepackage{{color}}\\color{{{2}}}\\dpi{{{3}}}\\normalsize {4}\" -o {5}") %
                "abc" % "red" % 45 % "asdf" % "asdf.png");
        CPPUNIT_ASSERT_EQUAL(
                std::string("abc -m 0 \"\\png\\usepackage{color}\\color{red}\\dpi{45}\\normalsize asdf\" -o asdf.png"),
                command);
    }

    void test3() {
        string msg = FS(FORMAT_STR(" of {1}{2}") % 5 % 6);
        CPPUNIT_ASSERT_EQUAL(std::string(" of 56"), msg);
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(I18nTest);
