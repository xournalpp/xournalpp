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

#include <cppunit/extensions/HelperMacros.h>

#include "../../control/LoadHandler.h"

using namespace std;

class LoadHandlerTest: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(LoadHandlerTest);
	CPPUNIT_TEST(testLoad1);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(LoadHandlerTest);
