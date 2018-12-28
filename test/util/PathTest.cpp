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
#include <Path.h>

#include <cppunit/extensions/HelperMacros.h>
#include <ctime>
#include <stdlib.h>

using namespace std;

class PathTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(PathTest);

	CPPUNIT_TEST(testUnsupportedUri);
	CPPUNIT_TEST(testPathFromUri);
	CPPUNIT_TEST(testParentPath);
	CPPUNIT_TEST(testFilename);
	CPPUNIT_TEST(testHasExtension);
	CPPUNIT_TEST(testClearExtensions);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testUnsupportedUri()
	{
		Path b = Path::fromUri("http://localhost/test.txt");
		CPPUNIT_ASSERT_EQUAL(true, b.isEmpty());
	}

	void testPathFromUri()
	{
		Path b = Path::fromUri("file:///tmp/test.txt");
		CPPUNIT_ASSERT_EQUAL(false, b.isEmpty());
		CPPUNIT_ASSERT_EQUAL(string("/tmp/test.txt"), b.str());
	}

	void testParentPath()
	{
		Path a = Path("C:\\test\\abc\\xyz.txt");
		CPPUNIT_ASSERT_EQUAL(string("C:\\test\\abc"), a.getParentPath().str());
		CPPUNIT_ASSERT_EQUAL(string("C:\\test"), a.getParentPath().getParentPath().str());

		Path b = Path("/temp/test/asdf.txt");
		CPPUNIT_ASSERT_EQUAL(string("/temp/test"), b.getParentPath().str());
		CPPUNIT_ASSERT_EQUAL(string("/temp"), b.getParentPath().getParentPath().str());
	}

	void testFilename()
	{
		Path a = Path("C:\\test\\abc\\xyz.txt");
		CPPUNIT_ASSERT_EQUAL(string("xyz.txt"), a.getFilename());
		CPPUNIT_ASSERT_EQUAL(string("abc"), a.getParentPath().getFilename());

		Path b = Path("/temp/test/asdf.txt");
		CPPUNIT_ASSERT_EQUAL(string("asdf.txt"), b.getFilename());
		CPPUNIT_ASSERT_EQUAL(string("test"), b.getParentPath().getFilename());
	}

	void testHasExtension()
	{
		Path a = Path("C:\\test\\abc\\xyz.txt");
		CPPUNIT_ASSERT_EQUAL(true, a.hasExtension(".txt"));

		Path b = Path("C:\\test\\abc\\xyz.TXT");
		CPPUNIT_ASSERT_EQUAL(true, b.hasExtension(".txt"));

		CPPUNIT_ASSERT_EQUAL(false, a.hasExtension("."));
		CPPUNIT_ASSERT_EQUAL(false, a.hasExtension("xyz"));
	}

	void testClearExtensions()
	{
		Path a = Path("C:\\test\\abc\\xyz.txt");
		a.clearExtensions();
		CPPUNIT_ASSERT_EQUAL(string("C:\\test\\abc\\xyz"), a.str());

		Path b = Path("/test/asdf.TXT");
		b.clearExtensions();
		CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.str());

		b = Path("/test/asdf.asdf/asdf");
		b.clearExtensions();
		CPPUNIT_ASSERT_EQUAL(string("/test/asdf.asdf/asdf"), b.str());
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);
