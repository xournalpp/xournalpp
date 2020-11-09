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

#include <StringUtils.h>
#include <config-test.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class StringUtilsTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(StringUtilsTest);

    CPPUNIT_TEST(testStartWith);
    CPPUNIT_TEST(testSplit);
    CPPUNIT_TEST(testSplitEmpty);
    CPPUNIT_TEST(testSplitOne);
    CPPUNIT_TEST(testEndsWith);
    CPPUNIT_TEST(testCompare);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

    void testStartWith() {
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::startsWith("asdfsfdafdasfda", "asdf"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("111111111111111", "2222"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("122221111111111", "2222"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("", "asdf"));
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::startsWith("aaaaaaa", ""));
    }

    void testSplit() {
        vector<string> splitted = StringUtils::split("a,,b,c,d,e,f", ',');

        CPPUNIT_ASSERT_EQUAL(7, (int)splitted.size());
        CPPUNIT_ASSERT_EQUAL(std::string("a"), splitted[0]);
        CPPUNIT_ASSERT_EQUAL(std::string(""), splitted[1]);
    }

    void testSplitEmpty() {
        vector<string> splitted = StringUtils::split("", ',');

        CPPUNIT_ASSERT_EQUAL(0, (int)splitted.size());
    }

    void testSplitOne() {
        vector<string> splitted = StringUtils::split("aa", ',');

        CPPUNIT_ASSERT_EQUAL(1, (int)splitted.size());
        CPPUNIT_ASSERT_EQUAL(std::string("aa"), splitted[0]);
    }

    void testEndsWith() {
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::endsWith("asdfsfdafdasfda.xoj", ".xoj"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::endsWith("111111111111111", "2222"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::endsWith("111111111122221", "2222"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::endsWith("", "asdf"));
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::endsWith("aaaaaaa", ""));
    }

    void testCompare() {
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::iequals("", ""));
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::iequals("aaaaaaaa", "aAAAaaaa"));
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::iequals("äää", "ÄÄÄ"));
        CPPUNIT_ASSERT_EQUAL(true, StringUtils::iequals("ööaa", "Ööaa"));
        CPPUNIT_ASSERT_EQUAL(false, StringUtils::iequals("ööaa", "ööaaa"));
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(StringUtilsTest);
