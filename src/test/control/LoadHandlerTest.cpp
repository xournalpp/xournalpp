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

#include "control/LoadHandler.h"

#include <cppunit/extensions/HelperMacros.h>

#include <ctime>
using std::clock;
#include <stdlib.h>

class LoadHandlerTest: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LoadHandlerTest);

	CPPUNIT_TEST(testSpeed);

	CPPUNIT_TEST(testLoad1);
	CPPUNIT_TEST(testLoad1Unzipped);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void printMemory()
	{
		int pid = ::getpid();

		char buffer[512];
		snprintf(buffer, sizeof(buffer), "bash -c \"cat /proc/%i/status | grep Vm\"", pid);

		system(buffer);
	}

	void testSpeed()
	{
		printf("== Speed ==\n");
		clock_t begin = clock();

		LoadHandler handler;
		Document* doc = handler.loadDocument("testfiles/big-test2.xoj");

		clock_t end = clock();

		printMemory();

		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		printf("Time to load document NEW: %lf\n", elapsed_secs);
	}

	void testLoad1()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument("testfiles/test1.xoj");

		CPPUNIT_ASSERT_EQUAL(1, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(1, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;

		CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
	}

	void testLoad1Unzipped()
	{
		/*
		LoadHandler handler;
		Document* doc = handler.loadDocument("testfiles/test1.unzipped.xoj");

		CPPUNIT_ASSERT_EQUAL(1, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(1, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;

		CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
		*/
	}
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(LoadHandlerTest);
