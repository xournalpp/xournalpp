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

#include <config-test.h>
#include <StringUtils.h>

#include <cppunit/extensions/HelperMacros.h>
#include <ctime>
#include <stdlib.h>

using namespace std;

class StringUtilTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(StringUtilTest);

	CPPUNIT_TEST(testStartWith);
	CPPUNIT_TEST(testSplit);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testStartWith()
	{
		CPPUNIT_ASSERT_EQUAL(true, StringUtils::startsWith("asdfsfdafdasfda", "asdf"));
		CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("111111111111111", "2222"));
		CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("122221111111111", "2222"));
		CPPUNIT_ASSERT_EQUAL(false, StringUtils::startsWith("", "asdf"));
		CPPUNIT_ASSERT_EQUAL(true, StringUtils::startsWith("aaaaaaa", ""));
	}

	void testSplit()
	{
		vector<string> splitted = StringUtils::split("a,,b,c,d,e,f", ',');

		CPPUNIT_ASSERT_EQUAL(8, (int)splitted.size());
		CPPUNIT_ASSERT_EQUAL(std::string("a"), splitted[0]);
		CPPUNIT_ASSERT_EQUAL(std::string(""), splitted[1]);
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(StringUtilTest);
