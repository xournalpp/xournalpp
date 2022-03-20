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

#include <cmath>
#include <filesystem>
#include <iostream>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/SaveHandler.h"
#include "util/PathUtil.h"

#include "filesystem.h"

using std::string;

// Common test Functions
void testLoadStoreLoad() {
    auto getElements = [](Document* doc) {
        EXPECT_EQ((size_t)1, doc->getPageCount());
        PageRef page = doc->getPage(0);

        EXPECT_EQ((size_t)1, (*page).getLayerCount());
        Layer* layer = (*(*page).getLayers())[0];

        const std::vector<Element*>& elements = layer->getElements();
        EXPECT_EQ(8, static_cast<int>(layer->getElements().size()));

        Stroke* e0 = (Stroke*)layer->getElements()[0];
        EXPECT_EQ(ELEMENT_STROKE, e0->getType());

        Stroke* e1 = (Stroke*)layer->getElements()[1];
        EXPECT_EQ(ELEMENT_STROKE, e1->getType());

        Stroke* e2 = (Stroke*)layer->getElements()[2];
        EXPECT_EQ(ELEMENT_STROKE, e2->getType());

        Stroke* e3 = (Stroke*)layer->getElements()[3];
        EXPECT_EQ(ELEMENT_STROKE, e3->getType());

        Stroke* e4 = (Stroke*)layer->getElements()[4];
        EXPECT_EQ(ELEMENT_STROKE, e4->getType());

        Text* e5 = (Text*)layer->getElements()[5];
        EXPECT_EQ(ELEMENT_TEXT, e5->getType());

        Stroke* e6 = (Stroke*)layer->getElements()[6];
        EXPECT_EQ(ELEMENT_STROKE, e6->getType());

        Stroke* e7 = (Stroke*)layer->getElements()[7];
        EXPECT_EQ(ELEMENT_STROKE, e7->getType());

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

    for (unsigned long i = 0; i < elements1.size(); i++) {
        Element* a = elements1.at(i);
        Element* b = elements2.at(i);
        EXPECT_EQ(a->getType(), b->getType());
        EXPECT_TRUE(coordEq(a->getX(), b->getX()));
        EXPECT_TRUE(coordEq(a->getY(), b->getY()));
        EXPECT_TRUE(coordEq(a->getElementWidth(), b->getElementWidth()));
        EXPECT_TRUE(coordEq(a->getElementHeight(), b->getElementHeight()));
        EXPECT_EQ(a->getColor(), b->getColor());
        switch (a->getType()) {
            case ELEMENT_STROKE: {
                auto sA = dynamic_cast<Stroke*>(a);
                auto sB = dynamic_cast<Stroke*>(b);
                EXPECT_EQ(sA->getPointCount(), sB->getPointCount());
                EXPECT_EQ(sA->getToolType(), sB->getToolType());
                EXPECT_EQ(sA->getLineStyle().hasDashes(), sB->getLineStyle().hasDashes());
                EXPECT_TRUE(coordEq(sA->getAvgPressure(), sB->getAvgPressure()));
                for (int j = 0; j < sA->getPointCount(); j++) {
                    Point pA = sA->getPoint(j);
                    Point pB = sB->getPoint(j);
                    EXPECT_TRUE(coordEq(pA.x, pB.x));
                    EXPECT_TRUE(coordEq(pA.y, pB.y));
                    EXPECT_TRUE(coordEq(pA.z, pB.z));
                }
                break;
            }
            case ELEMENT_TEXT: {
                auto tA = dynamic_cast<Text*>(a);
                auto tB = dynamic_cast<Text*>(b);
                EXPECT_EQ(tA->getText(), tB->getText());
                EXPECT_EQ(tA->getFontSize(), tB->getFontSize());
                break;
            }
            default:
                // If other elements are to be used in the test, implement extra comparisons
                EXPECT_TRUE(false);
                break;
        }
    }
}

void checkPageType(Document* doc, int pageIndex, string expectedText, PageType expectedBgType) {
    PageRef page = doc->getPage(pageIndex);

    PageType bgType = page->getBackgroundType();
    EXPECT_TRUE(expectedBgType == bgType);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Element* element = layer->getElements().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    Text* text = (Text*)element;
    EXPECT_EQ(expectedText, text->getText());
}


void checkLayer(PageRef page, int layerIndex, string expectedText) {
    Layer* layer = (*(*page).getLayers())[layerIndex];

    Element* element = layer->getElements().front();

    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    Text* text = (Text*)element;
    EXPECT_EQ(expectedText, text->getText());
}

TEST(ControlLoadHandler, testLoad) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("test1.xoj"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Element* element = layer->getElements().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    Text* text = (Text*)element;

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testLoadZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/test.xopp"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Element* element = layer->getElements().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    Text* text = (Text*)element;

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testLoadUnzipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("test1.unzipped.xoj"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Element* element = layer->getElements().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    Text* text = (Text*)element;

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testPages) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

    EXPECT_EQ((size_t)6, doc->getPageCount());
}

TEST(ControlLoadHandler, testPagesZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/pages.xopp"));

    EXPECT_EQ((size_t)6, doc->getPageCount());
}


TEST(ControlLoadHandler, testPageType) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("load/pages.xoj"));

    EXPECT_EQ((size_t)6, doc->getPageCount());
    checkPageType(doc, 0, "p1", PageType(PageTypeFormat::Plain));
    checkPageType(doc, 1, "p2", PageType(PageTypeFormat::Ruled));
    checkPageType(doc, 2, "p3", PageType(PageTypeFormat::Lined));
    checkPageType(doc, 3, "p4", PageType(PageTypeFormat::Staves));
    checkPageType(doc, 4, "p5", PageType(PageTypeFormat::Graph));
    checkPageType(doc, 5, "p6", PageType(PageTypeFormat::Image));
}

TEST(ControlLoadHandler, testPageTypeZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/pages.xopp"));

    EXPECT_EQ((size_t)6, doc->getPageCount());
    checkPageType(doc, 0, "p1", PageType(PageTypeFormat::Plain));
    checkPageType(doc, 1, "p2", PageType(PageTypeFormat::Ruled));
    checkPageType(doc, 2, "p3", PageType(PageTypeFormat::Lined));
    checkPageType(doc, 3, "p4", PageType(PageTypeFormat::Staves));
    checkPageType(doc, 4, "p5", PageType(PageTypeFormat::Graph));
    checkPageType(doc, 5, "p6", PageType(PageTypeFormat::Image));
}


TEST(ControlLoadHandler, testLayer) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("load/layer.xoj"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)3, (*page).getLayerCount());
    checkLayer(page, 0, "l1");
    checkLayer(page, 1, "l2");
    checkLayer(page, 2, "l3");
}

TEST(ControlLoadHandler, testLayerZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/layer.xopp"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)3, (*page).getLayerCount());
    checkLayer(page, 0, "l1");
    checkLayer(page, 1, "l2");
    checkLayer(page, 2, "l3");
}

TEST(ControlLoadHandler, testText) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("load/text.xml"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Text* t1 = (Text*)layer->getElements()[0];
    EXPECT_EQ(ELEMENT_TEXT, t1->getType());

    Text* t2 = (Text*)layer->getElements()[1];
    EXPECT_EQ(ELEMENT_TEXT, t2->getType());

    Text* t3 = (Text*)layer->getElements()[2];
    EXPECT_EQ(ELEMENT_TEXT, t3->getType());

    EXPECT_EQ(string("red"), t1->getText());
    EXPECT_EQ(string("blue"), t2->getText());
    EXPECT_EQ(string("green"), t3->getText());

    EXPECT_EQ(Color(0xff0000U), t1->getColor());
    EXPECT_EQ(Color(0x3333CCU), t2->getColor());
    EXPECT_EQ(Color(0x00f000U), t3->getColor());
}

TEST(ControlLoadHandler, testTextZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/text.xopp"));

    EXPECT_EQ((size_t)1, doc->getPageCount());
    PageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, (*page).getLayerCount());
    Layer* layer = (*(*page).getLayers())[0];

    Text* t1 = (Text*)layer->getElements()[0];
    EXPECT_EQ(ELEMENT_TEXT, t1->getType());

    Text* t2 = (Text*)layer->getElements()[1];
    EXPECT_EQ(ELEMENT_TEXT, t2->getType());

    Text* t3 = (Text*)layer->getElements()[2];
    EXPECT_EQ(ELEMENT_TEXT, t3->getType());

    EXPECT_EQ(string("red"), t1->getText());
    EXPECT_EQ(string("blue"), t2->getText());
    EXPECT_EQ(string("green"), t3->getText());

    EXPECT_EQ(Color(0xff0000U), t1->getColor());
    EXPECT_EQ(Color(0x3333CCU), t2->getColor());
    EXPECT_EQ(Color(0x00f000U), t3->getColor());
}

TEST(ControlLoadHandler, testImageZipped) {
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/imgAttachment/new.xopp"));

    EXPECT_EQ(1U, doc->getPageCount());
    PageRef page = doc->getPage(0);
    EXPECT_EQ(1U, page->getLayerCount());
    Layer* layer = (*page->getLayers())[0];
    EXPECT_EQ(layer->getElements().size(), 1);

    Image* img = dynamic_cast<Image*>(layer->getElements()[0]);
    EXPECT_TRUE(img);
}

namespace {
void checkImageFormat(Image* img, const char* formatName) {
    GdkPixbufLoader* imgLoader = gdk_pixbuf_loader_new();
    ASSERT_TRUE(gdk_pixbuf_loader_write(imgLoader, img->getRawData(), img->getRawDataLength(), nullptr));
    ASSERT_TRUE(gdk_pixbuf_loader_close(imgLoader, nullptr));
    GdkPixbufFormat* format = gdk_pixbuf_loader_get_format(imgLoader);
    ASSERT_TRUE(format) << "could not determine image format";
    EXPECT_STREQ(gdk_pixbuf_format_get_name(format), formatName);
}
}  // namespace

TEST(ControlLoadHandler, imageLoadJpeg) {
    // check loading of arbitrary image format (up to whatever is supported by GdkPixbuf)
    LoadHandler handler;
    Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/imgAttachment/doc_with_jpg.xopp"));
    ASSERT_TRUE(doc) << "doc should not be null";
    ASSERT_EQ(1U, doc->getPageCount());
    PageRef page = doc->getPage(0);
    ASSERT_EQ(1U, page->getLayerCount());
    Layer* layer = (*page->getLayers())[0];
    ASSERT_EQ(layer->getElements().size(), 1);

    Image* img = dynamic_cast<Image*>(layer->getElements()[0]);
    ASSERT_TRUE(img) << "element should be an image";

    checkImageFormat(img, "jpeg");
}

// FIXME: create a SaveHandlerTest.cpp and move this test here
TEST(ControlLoadHandler, imageSaveJpegBackwardCompat) {
    // File format version <= 4 requires images to be encoded as PNG in base64, but the version has not been bumped yet.
    // For backward compatibility, check that loaded JPEG images are saved in PNG format.

    // FIXME: use a path in CMAKE_BINARY_DIR or CMAKE_CURRENT_BINARY_DIR
    const fs::path outPath = fs::temp_directory_path() / "xournalpp-test-units_ControlLoaderHandler_imageLoadJpeg.xopp";

    // save journal containing JPEG image
    {
        LoadHandler handler;
        Document* doc = handler.loadDocument(GET_TESTFILE("packaged_xopp/imgAttachment/doc_with_jpg.xopp"));
        ASSERT_TRUE(doc) << "doc with jpeg should not be null";

        SaveHandler saver;
        saver.prepareSave(doc);
        saver.saveTo(outPath);
    }

    // check that the image is saved as PNG
    LoadHandler handler;
    Document* doc = handler.loadDocument(outPath);
    ASSERT_TRUE(doc) << "saved doc should not be null";
    ASSERT_EQ(1U, doc->getPageCount());
    PageRef page = doc->getPage(0);
    ASSERT_EQ(1U, page->getLayerCount());
    Layer* layer = (*page->getLayers())[0];
    ASSERT_EQ(layer->getElements().size(), 1);

    Image* img = dynamic_cast<Image*>(layer->getElements()[0]);
    ASSERT_TRUE(img) << "element should be an image";
    checkImageFormat(img, "png");
}

TEST(ControlLoadHandler, testLoadStoreLoadDefault) { testLoadStoreLoad(); }


#ifdef __linux__
TEST(ControlLoadHandler, testLoadStoreLoadGerman) {
    constexpr auto testLocale = "de_DE.UTF-8";
    char* currentLocale = setlocale(LC_ALL, testLocale);
    if (currentLocale == nullptr) {
        auto environ = g_get_environ();
        bool isCI = g_environ_getenv(environ, "CI");
        g_strfreev(environ);
        if (isCI) {
            EXPECT_TRUE(false);
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
