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

#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/SaveHandler.h"
#include "util/PathUtil.h"

#ifdef TEST_CHECK_SPEED
#include "SpeedTest.cpp"
#endif

#include <cmath>
#include <iostream>

#include <cppunit/extensions/HelperMacros.h>

#include "filesystem.h"

class LoadHandlerTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(LoadHandlerTest);

#ifdef TEST_CHECK_SPEED
    CPPUNIT_TEST(testSpeed);
#endif

    CPPUNIT_TEST(testLoad);
    CPPUNIT_TEST(testLoadZipped);
    CPPUNIT_TEST(testLoadUnzipped);

    CPPUNIT_TEST(testPages);
    CPPUNIT_TEST(testPagesZipped);
    CPPUNIT_TEST(testPageType);
    CPPUNIT_TEST(testPageTypeZipped);
    CPPUNIT_TEST(testLayer);
    CPPUNIT_TEST(testLayerZipped);
    CPPUNIT_TEST(testText);
    CPPUNIT_TEST(testTextZipped);
    CPPUNIT_TEST(testStroke);
    CPPUNIT_TEST(loadImage);
    CPPUNIT_TEST(testLoadStoreLoad);

#ifdef __linux__
    CPPUNIT_TEST(testLoadStoreLoadGerman);
#endif

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

#ifdef TEST_CHECK_SPEED
    void testSpeed() {
        SpeedTest speed;
        speed.startTest("document load");

        LoadHandler handler;
        handler.loadDocument(GET_TESTFILE("big-test.xoj"));

        speed.endTest();
    }

    void testSpeedZipped() {
        SpeedTest speed;
        speed.startTest("document load");

        LoadHandler handler;
        handler.loadDocument(GET_TESTFILE("packaged_xopp/big-test.xopp"));

        speed.endTest();
    }
#endif

    void testLoad() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("test1.xoj"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
        Layer* layer = (*(*page).getLayers())[0];

        Element* element = (*layer->getElements())[0];
        CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

        Text* text = (Text*)element;

        CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
    }

    void testLoadZipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/test.xopp"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
        Layer* layer = (*(*page).getLayers())[0];

        Element* element = (*layer->getElements())[0];
        CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

        Text* text = (Text*)element;

        CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
    }

    void testLoadUnzipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("test1.unzipped.xoj"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
        Layer* layer = (*(*page).getLayers())[0];

        Element* element = (*layer->getElements())[0];
        CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

        Text* text = (Text*)element;

        CPPUNIT_ASSERT_EQUAL(string("12345"), text->getText());
    }

    void testPages() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

        CPPUNIT_ASSERT_EQUAL((size_t)6, doc->getPageCount());
    }

    void testPagesZipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/pages.xopp"));

        CPPUNIT_ASSERT_EQUAL((size_t)6, doc->getPageCount());
    }

    void checkPageType(Document* doc, int pageIndex, string expectedText, PageType expectedBgType) {
        PageRef page = doc->getPage(pageIndex);

        PageType bgType = page->getBackgroundType();
        CPPUNIT_ASSERT(expectedBgType == bgType);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
        Layer* layer = (*(*page).getLayers())[0];

        Element* element = (*layer->getElements())[0];
        CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

        Text* text = (Text*)element;
        CPPUNIT_ASSERT_EQUAL(expectedText, text->getText());
    }

    void testPageType() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

        CPPUNIT_ASSERT_EQUAL((size_t)6, doc->getPageCount());
        checkPageType(doc, 0, "p1", PageType(PageTypeFormat::Plain));
        checkPageType(doc, 1, "p2", PageType(PageTypeFormat::Ruled));
        checkPageType(doc, 2, "p3", PageType(PageTypeFormat::Lined));
        checkPageType(doc, 3, "p4", PageType(PageTypeFormat::Staves));
        checkPageType(doc, 4, "p5", PageType(PageTypeFormat::Graph));
        checkPageType(doc, 5, "p6", PageType(PageTypeFormat::Image));
    }

    void testPageTypeZipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/pages.xopp"));

        CPPUNIT_ASSERT_EQUAL((size_t)6, doc->getPageCount());
        checkPageType(doc, 0, "p1", PageType(PageTypeFormat::Plain));
        checkPageType(doc, 1, "p2", PageType(PageTypeFormat::Ruled));
        checkPageType(doc, 2, "p3", PageType(PageTypeFormat::Lined));
        checkPageType(doc, 3, "p4", PageType(PageTypeFormat::Staves));
        checkPageType(doc, 4, "p5", PageType(PageTypeFormat::Graph));
        checkPageType(doc, 5, "p6", PageType(PageTypeFormat::Image));
    }

    void checkLayer(PageRef page, int layerIndex, string expectedText) {
        Layer* layer = (*(*page).getLayers())[layerIndex];

        Element* element = (*layer->getElements())[0];
        CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, element->getType());

        Text* text = (Text*)element;
        CPPUNIT_ASSERT_EQUAL(expectedText, text->getText());
    }

    void testLayer() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("load/layer.xoj"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)3, (*page).getLayerCount());
        checkLayer(page, 0, "l1");
        checkLayer(page, 1, "l2");
        checkLayer(page, 2, "l3");
    }

    void testLayerZipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/layer.xopp"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)3, (*page).getLayerCount());
        checkLayer(page, 0, "l1");
        checkLayer(page, 1, "l2");
        checkLayer(page, 2, "l3");
    }

    void testText() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("load/text.xml"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
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

        CPPUNIT_ASSERT_EQUAL(0xff0000U, t1->getColor());
        CPPUNIT_ASSERT_EQUAL(0x3333CCU, t2->getColor());
        CPPUNIT_ASSERT_EQUAL(0x00f000U, t3->getColor());
    }

    void testTextZipped() {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/text.xopp"));

        CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
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

        CPPUNIT_ASSERT_EQUAL(0xff0000U, t1->getColor());
        CPPUNIT_ASSERT_EQUAL(0x3333CCU, t2->getColor());
        CPPUNIT_ASSERT_EQUAL(0x00f000U, t3->getColor());
    }

    void testStroke() {}

    void loadImage() {}

    void testLoadStoreLoad() {
        auto getElements = [](Document* doc) {
            CPPUNIT_ASSERT_EQUAL((size_t)1, doc->getPageCount());
            PageRef page = doc->getPage(0);

            CPPUNIT_ASSERT_EQUAL((size_t)1, (*page).getLayerCount());
            Layer* layer = (*(*page).getLayers())[0];

            std::vector<Element*>* elements = layer->getElements();
            CPPUNIT_ASSERT_EQUAL(8, static_cast<int>(layer->getElements()->size()));

            Stroke* e0 = (Stroke*)(*layer->getElements())[0];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e0->getType());

            Stroke* e1 = (Stroke*)(*layer->getElements())[1];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e1->getType());

            Stroke* e2 = (Stroke*)(*layer->getElements())[2];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e2->getType());

            Stroke* e3 = (Stroke*)(*layer->getElements())[3];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e3->getType());

            Stroke* e4 = (Stroke*)(*layer->getElements())[4];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e4->getType());

            Text* e5 = (Text*)(*layer->getElements())[5];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_TEXT, e5->getType());

            Stroke* e6 = (Stroke*)(*layer->getElements())[6];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e6->getType());

            Stroke* e7 = (Stroke*)(*layer->getElements())[7];
            CPPUNIT_ASSERT_EQUAL(ELEMENT_STROKE, e7->getType());

            return elements;
        };
        LoadHandler handler;
        Document* doc1 = handler.loadDocument(GET_TESTFILE("packaged_xopp/suite.xopp"));
        auto elements1 = getElements(doc1);

        SaveHandler h;
        h.prepareSave(doc1);
        auto tmp = Util::getTmpDirSubfolder() / "save.xopp";
        h.saveTo(tmp);

        // Create a second loader so the first one doesn't free the memory
        LoadHandler handler2;
        Document* doc2 = handler2.loadDocument(tmp);
        auto elements2 = getElements(doc2);

        // Check that the coordinates from both files don't differ more than the precision they were saved with
        auto coordEq = [](double a, double b) { return std::abs(a - b) <= 1e-8; };

        for (unsigned long i = 0; i < elements1->size(); i++) {
            Element* a = elements1->at(i);
            Element* b = elements2->at(i);
            CPPUNIT_ASSERT_EQUAL(a->getType(), b->getType());
            CPPUNIT_ASSERT(coordEq(a->getX(), b->getX()));
            CPPUNIT_ASSERT(coordEq(a->getY(), b->getY()));
            CPPUNIT_ASSERT(coordEq(a->getElementWidth(), b->getElementWidth()));
            CPPUNIT_ASSERT(coordEq(a->getElementHeight(), b->getElementHeight()));
            CPPUNIT_ASSERT_EQUAL(a->getColor(), b->getColor());
            switch (a->getType()) {
                case ELEMENT_STROKE: {
                    auto sA = dynamic_cast<Stroke*>(a);
                    auto sB = dynamic_cast<Stroke*>(b);
                    CPPUNIT_ASSERT_EQUAL(sA->getPointCount(), sB->getPointCount());
                    CPPUNIT_ASSERT_EQUAL(sA->getToolType(), sB->getToolType());
                    CPPUNIT_ASSERT_EQUAL(sA->getLineStyle().hasDashes(), sB->getLineStyle().hasDashes());
                    CPPUNIT_ASSERT(coordEq(sA->getAvgPressure(), sB->getAvgPressure()));
                    for (int j = 0; j < sA->getPointCount(); j++) {
                        Point pA = sA->getPoint(j);
                        Point pB = sB->getPoint(j);
                        CPPUNIT_ASSERT(coordEq(pA.x, pB.x));
                        CPPUNIT_ASSERT(coordEq(pA.y, pB.y));
                        CPPUNIT_ASSERT(coordEq(pA.z, pB.z));
                    }
                    break;
                }
                case ELEMENT_TEXT: {
                    auto tA = dynamic_cast<Text*>(a);
                    auto tB = dynamic_cast<Text*>(b);
                    CPPUNIT_ASSERT_EQUAL(tA->getText(), tB->getText());
                    CPPUNIT_ASSERT_EQUAL(tA->getFontSize(), tB->getFontSize());
                    break;
                }
                default:
                    // If other elements are to be used in the test, implement extra comparisons
                    CPPUNIT_ASSERT(false);
                    break;
            }
        }
    }

#ifdef __linux__
    void testLoadStoreLoadGerman() {
        constexpr auto testLocale = "de_DE.UTF-8";
        char* currentLocale = setlocale(LC_ALL, testLocale);
        if (currentLocale == nullptr) {
            auto environ = g_get_environ();
            bool isCI = g_environ_getenv(environ, "CI");
            g_strfreev(environ);
            if (isCI) {
                CPPUNIT_ASSERT(false);
            } else {
                std::cout << "Skipping testLoadStoreLoadGerman! Consider generating the 'de_DE.UTF-8' locale on your "
                             "system."
                          << std::endl;
            }
        }
        testLoadStoreLoad();
        setlocale(LC_ALL, "C");
    }
#endif
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(LoadHandlerTest);
