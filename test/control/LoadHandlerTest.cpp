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

#include "control/xojfile/LoadHandler.h"
#include <config-test.h>

#ifdef TEST_CHECK_SPEED
#include "SpeedTest.cpp"
#endif

#include <cppunit/extensions/HelperMacros.h>

#include <stdlib.h>

class LoadHandlerTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(LoadHandlerTest);

#ifdef TEST_CHECK_SPEED
	CPPUNIT_TEST(testSpeed);
#endif

	CPPUNIT_TEST(testLoad1);
	CPPUNIT_TEST(testLoad1Unzipped);

	CPPUNIT_TEST(testPages);
	CPPUNIT_TEST(testPageType);
	CPPUNIT_TEST(testLayer);
	CPPUNIT_TEST(testText);
	CPPUNIT_TEST(testStroke);
	CPPUNIT_TEST(loadImage);

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

#ifdef TEST_CHECK_SPEED
	void testSpeed()
	{
		SpeedTest speed;
		speed.startTest("document load");
		
		LoadHandler handler;
		handler.loadDocument(GET_TESTFILE("big-test2.xoj"));
		
		speed.endTest();
	}
#endif

	void testLoad1()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("test1.xoj"));

		CPPUNIT_ASSERT_EQUAL(1UL, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(1UL, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;

		CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
	}

	void testLoad1Unzipped()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("test1.unzipped.xoj"));

		CPPUNIT_ASSERT_EQUAL(1UL, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(1UL, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;

		CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
	}

	void testPages()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

		CPPUNIT_ASSERT_EQUAL(5UL, doc->getPageCount());
	}

	void checkPageType(Document* doc, int pageIndex, string expectedText, BackgroundType expectedBgType)
	{
		PageRef page = doc->getPage(pageIndex);

		BackgroundType bgType = page->getBackgroundType();
		CPPUNIT_ASSERT_EQUAL(expectedBgType, bgType);

		CPPUNIT_ASSERT_EQUAL(1UL, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;
		CPPUNIT_ASSERT_EQUAL(expectedText, text->getText());
	}

	void testPageType()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

		CPPUNIT_ASSERT_EQUAL(5UL, doc->getPageCount());
		checkPageType(doc, 0, "p1", BACKGROUND_TYPE_NONE);
		checkPageType(doc, 1, "p2", BACKGROUND_TYPE_LINED);
		checkPageType(doc, 2, "p3", BACKGROUND_TYPE_RULED);
		checkPageType(doc, 3, "p4", BACKGROUND_TYPE_GRAPH);
		checkPageType(doc, 4, "p5", BACKGROUND_TYPE_IMAGE);

		// TODO: PDF not tested yet
		// BACKGROUND_TYPE_PDF
	}

	void checkLayer(PageRef page, int layerIndex, string expectedText)
	{
		Layer* layer = (*(*page).getLayers())[layerIndex];

		Element* element = (*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

		Text* text = (Text*) element;
		CPPUNIT_ASSERT_EQUAL(expectedText, text->getText());
	}

	void testLayer()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("load/layer.xoj"));

		CPPUNIT_ASSERT_EQUAL(1UL, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(3UL, (*page).getLayerCount());
		checkLayer(page, 0, "l1");
		checkLayer(page, 1, "l2");
		checkLayer(page, 2, "l3");
	}

	void testText()
	{
		LoadHandler handler;
		Document* doc = handler.loadDocument(GET_TESTFILE("load/text.xml"));

		CPPUNIT_ASSERT_EQUAL(1UL, doc->getPageCount());
		PageRef page = doc->getPage(0);

		CPPUNIT_ASSERT_EQUAL(1UL, (*page).getLayerCount());
		Layer* layer = (*(*page).getLayers())[0];

		Text* t1 = (Text*)(*layer->getElements())[0];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, t1->getType());

		Text* t2 = (Text*)(*layer->getElements())[1];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, t2->getType());

		Text* t3 = (Text*)(*layer->getElements())[2];
		CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, t3->getType());

		CPPUNIT_ASSERT_EQUAL(string("red"), t1->getText());
		CPPUNIT_ASSERT_EQUAL(string("blue"), t2->getText());
		CPPUNIT_ASSERT_EQUAL(string("green"), t3->getText());

		CPPUNIT_ASSERT_EQUAL(0xff0000, t1->getColor());
		CPPUNIT_ASSERT_EQUAL(0x3333CC, t2->getColor());
		CPPUNIT_ASSERT_EQUAL(0x00f000, t3->getColor());
	}

	void testStroke()
	{

	}

	void loadImage()
	{

	}

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(LoadHandlerTest);
