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
#include <XojPreviewExtractor.h>

#include <cppunit/extensions/HelperMacros.h>
#include <ctime>
#include <stdlib.h>

using namespace std;

class XojPreviewExtractorTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(XojPreviewExtractorTest);

	CPPUNIT_TEST(testNonExistingFile);
	CPPUNIT_TEST(testExtensionCheck);
	CPPUNIT_TEST(testLoadGzipped);
	CPPUNIT_TEST(testLoadGzipped2);
	CPPUNIT_TEST(testLoad1Unzipped);
	CPPUNIT_TEST(testNoPreview);
	CPPUNIT_TEST(testInvalidFile);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void testNonExistingFile()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("THIS FILE DOES NOT EXIST.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_COULD_NOT_OPEN_FILE, result);
	}

	void testExtensionCheck()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("THIS FILE DOES NOT EXIST.xoi"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_BAD_FILE_EXTENSION, result);
		
		result = extractor.readFile(GET_TESTFILE("THIS FILE DOES NOT EXIST.xOj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_COULD_NOT_OPEN_FILE, result);
	}

	void testLoadGzipped()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_IMAGE_READ, result);
		
		CPPUNIT_ASSERT_EQUAL(string("CppUnitTestString"), string(extractor.getData()));
	}

	void testLoadGzipped2()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test2.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_IMAGE_READ, result);

		CPPUNIT_ASSERT_EQUAL((std::string::size_type) 2856, extractor.getData().length());
	}

	void testLoad1Unzipped()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test.unzipped.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_IMAGE_READ, result);

		CPPUNIT_ASSERT_EQUAL(string("CppUnitTestString"), string(extractor.getData()));
	}

	void testNoPreview()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test-no-preview.unzipped.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_NO_PREVIEW, result);
	}

	void testInvalidFile()
	{
		XojPreviewExtractor extractor;
		PreviewExtractResult result = extractor.readFile(GET_TESTFILE("preview-test-invalid.xoj"));

		CPPUNIT_ASSERT_EQUAL(PREVIEW_RESULT_ERROR_READING_PREVIEW, result);
	}

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(XojPreviewExtractorTest);
