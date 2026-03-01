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

#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string_view>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/SaveHandler.h"
#include "model/Element.h"
#include "model/Image.h"
#include "model/PageRef.h"
#include "model/Stroke.h"
#include "model/StrokeStyle.h"
#include "model/TexImage.h"
#include "model/Text.h"
#include "model/XojPage.h"
#include "util/PathUtil.h"
#include "util/StringUtils.h"

#include "filesystem.h"

using std::string;

// Common test Functions
namespace {

/**
 * Load a test file, and verify that loading does not throw.
 */
std::unique_ptr<Document> loadTestDocument(const fs::path& filepath) {
    LoadHandler handler;
    EXPECT_NO_THROW(return handler.loadDocument(filepath)) << "Error while loading \"" << filepath << '\"';
    return {};
}

/**
 * Unit test implementation for the "suite.xopp" file and its derivatives.
 * \param filepath The path to the actual file to load.
 * \param tol The absolute tolerance used when checking stroke coordinate data.
 */
void testLoadStoreLoadHelper(const fs::path& filepath, double tol = 1e-8) {
    auto getElements = [](Document* doc) {
        EXPECT_EQ((size_t)1, doc->getPageCount());
        ConstPageRef page = doc->getPage(0);

        EXPECT_EQ((size_t)1, page->getLayerCount());
        const Layer* layer = page->getLayersView()[0];

        auto elements = layer->getElementsView();
        EXPECT_EQ(8, static_cast<int>(elements.size()));

        Stroke* e0 = (Stroke*)elements[0];
        EXPECT_EQ(ELEMENT_STROKE, e0->getType());

        Stroke* e1 = (Stroke*)elements[1];
        EXPECT_EQ(ELEMENT_STROKE, e1->getType());

        Stroke* e2 = (Stroke*)elements[2];
        EXPECT_EQ(ELEMENT_STROKE, e2->getType());

        Stroke* e3 = (Stroke*)elements[3];
        EXPECT_EQ(ELEMENT_STROKE, e3->getType());

        Stroke* e4 = (Stroke*)elements[4];
        EXPECT_EQ(ELEMENT_STROKE, e4->getType());

        Text* e5 = (Text*)elements[5];
        EXPECT_EQ(ELEMENT_TEXT, e5->getType());

        Stroke* e6 = (Stroke*)elements[6];
        EXPECT_EQ(ELEMENT_STROKE, e6->getType());

        Stroke* e7 = (Stroke*)elements[7];
        EXPECT_EQ(ELEMENT_STROKE, e7->getType());

        return elements;
    };
    auto doc1 = loadTestDocument(filepath);
    auto elements1 = getElements(doc1.get());

    SaveHandler h;
    auto tmp = Util::getTmpDirSubfolder() / "save.xopp";
    h.prepareSave(doc1.get(), tmp);
    h.saveTo(tmp);

    auto doc2 = loadTestDocument(tmp);
    auto elements2 = getElements(doc2.get());

    // Check that the coordinates from both files don't differ more than the precision they were saved with
    auto coordEq = [tol](double a, double b) { return std::abs(a - b) <= tol; };

    for (unsigned long i = 0; i < elements1.size(); i++) {
        const Element* a = elements1[i];
        const Element* b = elements2[i];
        EXPECT_EQ(a->getType(), b->getType());
        EXPECT_TRUE(coordEq(a->getX(), b->getX()));
        EXPECT_TRUE(coordEq(a->getY(), b->getY()));
        EXPECT_TRUE(coordEq(a->getElementWidth(), b->getElementWidth()));
        EXPECT_TRUE(coordEq(a->getElementHeight(), b->getElementHeight()));
        EXPECT_EQ(a->getColor(), b->getColor());
        switch (a->getType()) {
            case ELEMENT_STROKE: {
                auto sA = dynamic_cast<const Stroke*>(a);
                auto sB = dynamic_cast<const Stroke*>(b);
                EXPECT_EQ(sA->getPointCount(), sB->getPointCount());
                EXPECT_EQ(sA->getToolType(), sB->getToolType());
                EXPECT_EQ(sA->getLineStyle().hasDashes(), sB->getLineStyle().hasDashes());
                EXPECT_TRUE(coordEq(sA->getAvgPressure(), sB->getAvgPressure()));
                for (size_t j = 0; j < sA->getPointCount(); j++) {
                    Point pA = sA->getPoint(j);
                    Point pB = sB->getPoint(j);
                    EXPECT_TRUE(coordEq(pA.x, pB.x));
                    EXPECT_TRUE(coordEq(pA.y, pB.y));
                    EXPECT_TRUE(coordEq(pA.z, pB.z));
                }
                break;
            }
            case ELEMENT_TEXT: {
                auto tA = dynamic_cast<const Text*>(a);
                auto tB = dynamic_cast<const Text*>(b);
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

void checkPageType(const Document* doc, size_t pageIndex, const string& expectedText, const PageType& expectedBgType) {
    ASSERT_LT(pageIndex, doc->getPageCount());
    ConstPageRef page = doc->getPage(pageIndex);

    PageType bgType = page->getBackgroundType();
    EXPECT_EQ(expectedBgType, bgType);

    if (!bgType.isSpecial()) {
        EXPECT_EQ(page->getBackgroundColor(), Colors::white);
    }

    EXPECT_EQ((size_t)1, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];

    auto* element = layer->getElementsView().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    auto* text = dynamic_cast<const Text*>(element);
    EXPECT_NE(text, nullptr);
    EXPECT_EQ(expectedText, text->getText());
}

void checkLayer(ConstPageRef page, size_t layerIndex, const std::optional<std::string>& optName,
                const std::string& expectedText) {
    ASSERT_LT(layerIndex, page->getLayerCount());
    const Layer* layer = page->getLayersView()[layerIndex];

    EXPECT_EQ(layer->hasName(), optName.has_value());
    if (optName) {
        EXPECT_EQ(layer->getName(), *optName);
    }

    auto* element = layer->getElementsView().front();

    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    auto* text = dynamic_cast<const Text*>(element);
    EXPECT_NE(text, nullptr);
    EXPECT_EQ(expectedText, text->getText());
}

void checkStroke(const Layer* layer, size_t elementIndex, StrokeTool tool, Color color, double width, int fill,
                 StrokeCapStyle capStyle, const LineStyle& lineStyle,
                 const std::vector<std::pair<size_t, Point>>& pointSamples = {}) {
    ASSERT_LT(elementIndex, layer->getElementsView().size());
    const auto* stroke = dynamic_cast<const Stroke*>(layer->getElementsView()[elementIndex]);

    ASSERT_NE(stroke, nullptr) << "Element " << elementIndex << " should be a stroke";
    ASSERT_EQ(stroke->getType(), ELEMENT_STROKE) << "Element " << elementIndex << " should be a stroke";

    auto pointEq = [](const Point& a, const Point& b) {
        return (std::abs(a.x - b.x) <= std::max(std::abs(a.x), 1.0) * 1e-10) &&
               (std::abs(a.y - b.y) <= std::max(std::abs(a.y), 1.0) * 1e-10) &&
               (std::abs(a.z - b.z) <= std::max(std::abs(a.z), 1.0) * 1e-10);
    };

    EXPECT_EQ(stroke->getToolType(), tool) << "Stroke at index " << elementIndex << " has the wrong tool";
    EXPECT_EQ(stroke->getColor(), color) << "Stroke at index " << elementIndex << " has the wrong color";
    EXPECT_DOUBLE_EQ(stroke->getWidth(), width) << "Stroke at index " << elementIndex << " has the wrong width";
    EXPECT_EQ(stroke->getFill(), fill) << "Stroke at index " << elementIndex << " has the wrong fill";
    EXPECT_EQ(stroke->getStrokeCapStyle(), capStyle)
            << "Stroke at index " << elementIndex << " has the wrong stroke cap style";
    EXPECT_EQ(stroke->getLineStyle(), lineStyle)
            << "Stroke at index " << elementIndex << " has an incorrect line style";

    for (const auto& sample: pointSamples) {
        ASSERT_LT(sample.first, stroke->getPointCount());
        EXPECT_TRUE(pointEq(stroke->getPoint(sample.first), sample.second))
                << "Stroke at index " << elementIndex << " has incorrect points";
    }
}

void checkImageFormat(const Image* img, const char* formatName) {
    const auto gdkFormatName = gdk_pixbuf_format_get_name(img->getImageFormat());
    EXPECT_STREQ(gdkFormatName, formatName);
    g_free(gdkFormatName);
}

void checkText(const Layer* layer, size_t elementIndex, const std::string& text, Color color) {
    ASSERT_LT(elementIndex, layer->getElementsView().size());
    const auto* textElem = dynamic_cast<const Text*>(layer->getElementsView()[elementIndex]);

    ASSERT_NE(textElem, nullptr) << "Element " << elementIndex << " should be text";
    ASSERT_EQ(textElem->getType(), ELEMENT_TEXT) << "Element " << elementIndex << " should be text";

    EXPECT_EQ(textElem->getText(), text) << "Text at index " << elementIndex << " has incorrect contents";
    EXPECT_EQ(textElem->getColor(), color) << "Text at index " << elementIndex << " has the wrong color";
}

}  // namespace

TEST(ControlLoadHandler, testLoad) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"test1.xoj"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)1, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];

    auto* element = layer->getElementsView().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    auto* text = dynamic_cast<const Text*>(element);
    EXPECT_NE(text, nullptr);

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testLoadZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/test.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)1, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];

    auto* element = layer->getElementsView().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    auto* text = dynamic_cast<const Text*>(element);
    EXPECT_NE(text, nullptr);

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testLoadUnzipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"test1.unzipped.xoj"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)1, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];

    auto* element = layer->getElementsView().front();
    EXPECT_EQ(ELEMENT_TEXT, element->getType());

    auto* text = dynamic_cast<const Text*>(element);
    EXPECT_NE(text, nullptr);

    EXPECT_EQ(string("12345"), text->getText());
}

TEST(ControlLoadHandler, testPages) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/pages.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{11}, doc->getPageCount());

    // Check page dimensions
    constexpr double A4_WIDTH = 595.27559, A4_HEIGHT = 841.88976;
    for (int i = 0; i < 4; ++i) {
        EXPECT_DOUBLE_EQ(doc->getPage(i)->getWidth(), A4_WIDTH);
        EXPECT_DOUBLE_EQ(doc->getPage(i)->getHeight(), A4_HEIGHT);
    }
    EXPECT_DOUBLE_EQ(doc->getPage(10)->getWidth(), 50.0);
    EXPECT_DOUBLE_EQ(doc->getPage(10)->getHeight(), 50.0);
}

TEST(ControlLoadHandler, testPagesZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/pages.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)6, doc->getPageCount());
}


TEST(ControlLoadHandler, testPageType) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/pages.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{11}, doc->getPageCount());

    // Non-standard page types
    auto rightLined = PageType(PageTypeFormat::Lined);
    rightLined.config = "m1=-72";
    auto graphWithBorder = PageType(PageTypeFormat::Graph);
    graphWithBorder.config = "m1=40,rm=1";

    checkPageType(doc.get(), 0, "p1", PageType(PageTypeFormat::Plain));
    checkPageType(doc.get(), 1, "p2", PageType(PageTypeFormat::Ruled));
    checkPageType(doc.get(), 2, "p3", PageType(PageTypeFormat::Lined));
    checkPageType(doc.get(), 3, "p4", rightLined);
    checkPageType(doc.get(), 4, "p5", PageType(PageTypeFormat::Staves));
    checkPageType(doc.get(), 5, "p6", PageType(PageTypeFormat::Graph));
    checkPageType(doc.get(), 6, "p7", PageType(PageTypeFormat::Dotted));
    checkPageType(doc.get(), 7, "p8", PageType(PageTypeFormat::IsoDotted));
    checkPageType(doc.get(), 8, "p9", PageType(PageTypeFormat::IsoGraph));
    checkPageType(doc.get(), 9, "p10", graphWithBorder);
    checkPageType(doc.get(), 10, "p11", PageType(PageTypeFormat::Image));

    EXPECT_TRUE(doc->getPage(10)->getBackgroundImage().isAttached());
    EXPECT_EQ(doc->getPage(10)->getBackgroundImage().getFilepath().filename(), "pages.xopp.bg_1.png");
}

TEST(ControlLoadHandler, testPageTypeZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/pages.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)6, doc->getPageCount());
    checkPageType(doc.get(), 0, "p1", PageType(PageTypeFormat::Plain));
    checkPageType(doc.get(), 1, "p2", PageType(PageTypeFormat::Ruled));
    checkPageType(doc.get(), 2, "p3", PageType(PageTypeFormat::Lined));
    checkPageType(doc.get(), 3, "p4", PageType(PageTypeFormat::Staves));
    checkPageType(doc.get(), 4, "p5", PageType(PageTypeFormat::Graph));
    checkPageType(doc.get(), 5, "p6", PageType(PageTypeFormat::Image));

    EXPECT_EQ(doc->getPage(5)->getBackgroundImage().getFilepath(), "attachments/bg_1.png");
}

TEST(ControlLoadHandler, testPageTypeFormatCopyFix) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"pageTypeFormatCopy.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)3, doc->getPageCount());
    checkPageType(doc.get(), 0, "p1", PageType(PageTypeFormat::Lined));
    checkPageType(doc.get(), 1, "p2", PageType(PageTypeFormat::Plain));  // PageTypeFormat::Copy in the file
    checkPageType(doc.get(), 2, "p3", PageType(PageTypeFormat::Plain));  // PageTypeFormat::Plain in the file
}

TEST(ControlLoadHandler, testLayers) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/layers.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ((size_t)3, page->getLayerCount());
    checkLayer(page, 0, std::nullopt, "l1");
    checkLayer(page, 1, "Named layer", "l2");
    checkLayer(page, 2, std::nullopt, "l3");
}

TEST(ControlLoadHandler, testLayerZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/layer.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{3}, page->getLayerCount());
    checkLayer(page, 0, std::nullopt, "l1");
    checkLayer(page, 1, std::nullopt, "l2");
    checkLayer(page, 2, std::nullopt, "l3");
}

TEST(ControlLoadHandler, testStrokes) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/strokes.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{1}, page->getLayerCount());
    const Layer* layer = page->getLayersView().front();

    ASSERT_EQ(size_t{12}, layer->getElementsView().size());
    checkStroke(layer, 0, StrokeTool::PEN, Color(0xff002e99), 2.26, -1, StrokeCapStyle::ROUND, LineStyle{},
                {{0, {210.17715, 200.96557, 0.41932136}},
                 {30, {264.91445, 170.38327, 0.66099109}},
                 {67, {377.23607, 173.27295, 0.56020856}},
                 {68, {380.69162, 170.66197, Point::NO_PRESSURE}}});
    checkStroke(layer, 1, StrokeTool::PEN, Color(0xff808080), 2.26, -1, StrokeCapStyle::ROUND,
                StrokeStyle::parseStyle("dash"),
                {{0, {396.76, 226.72, Point::NO_PRESSURE}},
                 {1, {396.76, 141.7, Point::NO_PRESSURE}},
                 {2, {198.38, 141.7, Point::NO_PRESSURE}},
                 {3, {198.38, 226.72, Point::NO_PRESSURE}},
                 {4, {396.76, 226.72, Point::NO_PRESSURE}}});
    checkStroke(layer, 2, StrokeTool::PEN, Color(0xffed5353), 2.26, -1, StrokeCapStyle::ROUND,
                StrokeStyle::parseStyle("dashdot"));
    checkStroke(layer, 3, StrokeTool::PEN, Color(0xffffa154), 1.41, -1, StrokeCapStyle::ROUND,
                StrokeStyle::parseStyle("dot"),
                {{0, {86.46991, 248.29627, Point::NO_PRESSURE}}, {1, {518.99074, 248.29627, Point::NO_PRESSURE}}});
    checkStroke(layer, 4, StrokeTool::PEN, Colors::black, 1.41, -1, StrokeCapStyle::ROUND, LineStyle{});
    checkStroke(layer, 5, StrokeTool::ERASER, Colors::white, 8.5, -1, StrokeCapStyle::ROUND, LineStyle{});
    checkStroke(layer, 6, StrokeTool::PEN, Color(0xff64baff), 0.85, 128, StrokeCapStyle::ROUND, LineStyle{});
    checkStroke(layer, 7, StrokeTool::HIGHLIGHTER, Color(0x7fffa154), 19.84, -1, StrokeCapStyle::ROUND, LineStyle{});
    checkStroke(layer, 8, StrokeTool::HIGHLIGHTER, Color(0x7f9bdb4d), 2.83, -1, StrokeCapStyle::ROUND, LineStyle{});
    checkStroke(layer, 9, StrokeTool::PEN, Colors::black, 2.26, -1, StrokeCapStyle::ROUND, LineStyle{},
                {{0, {200.23669, 358.03824, Point::NO_PRESSURE}}, {1, {200.23669, 358.03824, Point::NO_PRESSURE}}});
    checkStroke(layer, 10, StrokeTool::HIGHLIGHTER, Color(0x7fffe16b), 10.616218, 255, StrokeCapStyle::BUTT,
                LineStyle{},
                {{0, {128.78249, 488.6327, Point::NO_PRESSURE}}, {1, {466.4931, 488.6327, Point::NO_PRESSURE}}});
    checkStroke(layer, 11, StrokeTool::HIGHLIGHTER, Color(0x7f000000), 1, 230, StrokeCapStyle::BUTT, LineStyle{},
                {{0, {143.58172, 506.94589, Point::NO_PRESSURE}}, {1, {451.69387, 506.94589, Point::NO_PRESSURE}}});
}

TEST(ControlLoadHandler, testText) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/text.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{1}, page->getLayerCount());
    const Layer* layer = page->getLayersView().front();

    checkText(layer, 0, "red", Colors::red);
    checkText(layer, 1, "blue", Colors::xopp_royalblue);
    checkText(layer, 2, "green", Color(0xff00f000U));
    checkText(layer, 3, "multiline\ntext", Colors::black);
    checkText(layer, 4, " \n odd  whitespace\ttext\n\n", Colors::black);
}

TEST(ControlLoadHandler, testTextZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/text.xopp"));
    ASSERT_TRUE(doc);

    ASSERT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{1}, page->getLayerCount());
    const Layer* layer = page->getLayersView().front();

    checkText(layer, 0, "red", Colors::red);
    checkText(layer, 1, "blue", Colors::xopp_royalblue);
    checkText(layer, 2, "green", Color(0xff00f000U));
}

TEST(ControlLoadHandler, testImage) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/image.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{1}, page->getLayerCount());
    const Layer* layer = page->getLayersView().front();

    ASSERT_EQ(layer->getElementsView().size(), 1);
    const auto* img = dynamic_cast<const Image*>(layer->getElementsView()[0]);

    ASSERT_NE(img, nullptr) << "Element should be an image";
    ASSERT_EQ(img->getType(), ELEMENT_IMAGE) << "Element should be an image";

    EXPECT_DOUBLE_EQ(img->getX(), 41.637795);
    EXPECT_DOUBLE_EQ(img->getY(), 164.94488);
    EXPECT_NEAR(img->getElementWidth(), 512.0, 1e-5);
    EXPECT_NEAR(img->getElementHeight(), 512.0, 1e-5);

    EXPECT_GT(img->getRawDataLength(), 0);
    EXPECT_NE(img->getRawData(), nullptr);
    checkImageFormat(img, "png");
}

TEST(ControlLoadHandler, testImageZipped) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/imgAttachment/new.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ(1U, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);
    EXPECT_EQ(1U, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];
    EXPECT_EQ(layer->getElementsView().size(), 1);

    const Image* img = dynamic_cast<const Image*>(layer->getElementsView()[0]);
    ASSERT_NE(img, nullptr) << "Element should be an image";
    ASSERT_EQ(img->getType(), ELEMENT_IMAGE) << "Element should be an image";

    EXPECT_GT(img->getRawDataLength(), 0);
    EXPECT_NE(img->getRawData(), nullptr);
    checkImageFormat(img, "png");
}

TEST(ControlLoadHandler, imageLoadJpeg) {
    // check loading of arbitrary image format (up to whatever is supported by GdkPixbuf)
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/imgAttachment/doc_with_jpg.xopp"));
    ASSERT_TRUE(doc) << "doc should not be null";

    ASSERT_EQ(1U, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);
    ASSERT_EQ(1U, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];
    ASSERT_EQ(layer->getElementsView().size(), 1);

    const Image* img = dynamic_cast<const Image*>(layer->getElementsView()[0]);
    ASSERT_TRUE(img) << "element should be an image";

    checkImageFormat(img, "jpeg");
}

// FIXME: create a SaveHandlerTest.cpp and move this test here
TEST(ControlLoadHandler, imageSaveJpegBackwardCompat) {
    // File format version <= 4 requires images to be encoded as PNG in base64, but the version has not been bumped yet.
    // For backward compatibility, check that loaded JPEG images are saved in PNG format.

    // FIXME: use a path in CMAKE_BINARY_DIR or CMAKE_CURRENT_BINARY_DIR
    const fs::path outPath = fs::temp_directory_path() / "xournalpp-test-units_ControlLoaderHandler_imageLoadJpeg.xopp";
    ASSERT_TRUE(!fs::exists(outPath));

    // save journal containing JPEG image
    {
        auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/imgAttachment/doc_with_jpg.xopp"));
        ASSERT_TRUE(doc) << "doc with jpeg should not be null";

        SaveHandler saver;
        saver.prepareSave(doc.get(), outPath);
        saver.saveTo(outPath);
    }

    // check that the image is saved as PNG
    auto doc = loadTestDocument(outPath);
    ASSERT_TRUE(doc) << "saved doc should not be null";
    ASSERT_EQ(1U, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);
    ASSERT_EQ(1U, page->getLayerCount());
    const Layer* layer = page->getLayersView()[0];
    ASSERT_EQ(layer->getElementsView().size(), 1);

    const Image* img = dynamic_cast<const Image*>(layer->getElementsView()[0]);
    ASSERT_TRUE(img) << "element should be an image";
    checkImageFormat(img, "png");

    fs::remove(outPath);
}

TEST(ControlLoadHandler, testLatex) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/latex.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ(size_t{1}, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    ASSERT_EQ(size_t{1}, page->getLayerCount());
    const Layer* layer = page->getLayersView().front();

    ASSERT_EQ(layer->getElementsView().size(), 1);
    const auto* teximage = dynamic_cast<const TexImage*>(layer->getElementsView()[0]);

    ASSERT_NE(teximage, nullptr) << "Element should be a TeX image";
    ASSERT_EQ(teximage->getType(), ELEMENT_TEXIMAGE) << "Element should be a TeX image";

    EXPECT_STREQ(teximage->getText().c_str(), "x^2") << "TeX image has wrong text contents";

    EXPECT_DOUBLE_EQ(teximage->getX(), 14.937);
    EXPECT_DOUBLE_EQ(teximage->getY(), 15.715);
    EXPECT_DOUBLE_EQ(teximage->getElementWidth(), 20.126);
    EXPECT_DOUBLE_EQ(teximage->getElementHeight(), 18.57);

    EXPECT_FALSE(teximage->getBinaryData().empty());
}

TEST(ControlLoadHandler, linebreaksLatex) {
    // FIXME: use a path in CMAKE_BINARY_DIR or CMAKE_CURRENT_BINARY_DIR
    const fs::path outPath =
            fs::temp_directory_path() / "xournalpp-test-units_ControlLoaderHandler_linebreaksLatex.xopp";
    ASSERT_TRUE(!fs::exists(outPath));

    // save journal containing latex object with linebreaks.
    {
        auto doc = loadTestDocument(GET_TESTFILE(u8"load/linebreaksLatex.xopp"));
        ASSERT_TRUE(doc) << "latex objects with linebreaks";

        SaveHandler saver;
        saver.prepareSave(doc.get(), outPath);
        saver.saveTo(outPath);
    }

    // check that the saved latex objects have correct linebreaks.
    auto doc = loadTestDocument(outPath);
    ASSERT_TRUE(doc) << "saved latex objects should have correct linebreaks";
    ASSERT_EQ(1U, doc->getPageCount());
    PageRef page = doc->getPage(0);
    ASSERT_EQ(1U, page->getLayerCount());
    Layer* layer = page->getLayers()[0];

    auto* teximage = static_cast<const TexImage*>(layer->getElements()[0].get());
    EXPECT_EQ("{.\n}", teximage->getText());

    teximage = static_cast<const TexImage*>(layer->getElements()[1].get());
    EXPECT_EQ("{.\r}", teximage->getText());

    teximage = static_cast<const TexImage*>(layer->getElements()[2].get());
    EXPECT_EQ("{.\r\n}", teximage->getText());

    fs::remove(outPath);
}

TEST(ControlLoadHandler, testLoadStoreLoadDefault) {
    testLoadStoreLoadHelper(GET_TESTFILE(u8"packaged_xopp/suite.xopp"), /*tol=*/1e-8);
}

// Backwards compatibility test that checks that full-precision float strings can be loaded.
// See https://github.com/xournalpp/xournalpp/pull/4065
TEST(ControlLoadHandler, testLoadStoreLoadFloatBwCompat) {
    testLoadStoreLoadHelper(GET_TESTFILE(u8"packaged_xopp/suite_float_bw_compat.xopp"), /*tol=*/1e-5);
}

TEST(ControlLoadHandler, testStrokeWidthRecovery) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"packaged_xopp/stroke/width_recovery.xopp"));
    ASSERT_TRUE(doc);

    EXPECT_EQ((size_t)1, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, page->getLayerCount());

    const Layer* layer = page->getLayersView()[0];

    EXPECT_EQ(9U, layer->getElementsView().size());

    auto* s1 = dynamic_cast<const Stroke*>(layer->getElementsView()[0]);
    EXPECT_NE(s1, nullptr);
    EXPECT_EQ(ELEMENT_STROKE, s1->getType());
    for (auto& p: s1->getPointVector()) {
        EXPECT_EQ(p.z, Point::NO_PRESSURE);
    }

    auto testPressureValues = [elts = layer->getElementsView()](size_t n, const std::vector<double>& pressures) {
        auto* s = static_cast<const Stroke*>(elts[n]);
        printf("Testing stroke %zu\n", n);
        EXPECT_EQ(ELEMENT_STROKE, s->getType());
        EXPECT_EQ(Colors::lime, s->getColor());
        EXPECT_EQ(1.41, s->getWidth());
        auto pts = s->getPointVector();
        EXPECT_EQ(pts.size(), pressures.size());
        EXPECT_EQ(std::mismatch(pressures.begin(), pressures.end(), pts.begin(),
                                [](double v, const Point& p) { return v == p.z; })
                          .first,
                  pressures.end());
    };

    // This stroke got its last point removed and a negative pressure value got straightened up
    testPressureValues(1, {0.16, 0.16, 0.20, 0.22, 0.26, 0.14, Point::NO_PRESSURE});

    // The stroke is split in 4 bits due to null pressure values at various places
    testPressureValues(2, {0.16, Point::NO_PRESSURE});

    testPressureValues(3, {0.28, 0.30, 0.34, 0.22, 0.18, Point::NO_PRESSURE});

    testPressureValues(4, {0.16, 0.16, 0.22, 0.28, 0.30, Point::NO_PRESSURE});

    testPressureValues(5, {0.30, 0.34, 0.34, 0.38, 0.40, 0.40, 0.42, 0.46, 0.46, 0.46, 0.50, 0.52, Point::NO_PRESSURE});

    testPressureValues(
            6, {0.56, 0.56, 0.58, 0.60, 0.56, 0.40, 0.32, 0.18, 0.12, 0.16, 0.16, 0.20, 0.22, Point::NO_PRESSURE});

    // The stroke is split in 2 bits due to "nan" pressure values at various places
    testPressureValues(7, {0.20, 0.30, 0.10, Point::NO_PRESSURE});

    testPressureValues(8, {0.25, 0.30, 0.40, Point::NO_PRESSURE});
}

TEST(ControlLoadHandler, testLoadStoreCJK) {
    LoadHandler handler;
    auto filepath = std::u8string(GET_TESTFILE(u8"cjk/测试.xopp"));
    auto doc = handler.loadDocument(fs::path(filepath));
    ASSERT_NE(doc.get(), nullptr);

    EXPECT_EQ(doc->getPdfFilepath().filename().u8string(), std::u8string_view{u8"测试.pdf"});

    EXPECT_EQ((size_t)2, doc->getPageCount());
    ConstPageRef page = doc->getPage(0);

    EXPECT_EQ((size_t)1, page->getLayerCount());
    const auto* layer = page->getLayersView()[0];

    auto elements = layer->getElementsView();
    ASSERT_EQ((size_t)3, layer->getElementsView().size());

    auto check_element = [&](size_t i, const auto* answer) {
        EXPECT_EQ(ELEMENT_TEXT, elements[i]->getType());
        auto* text = dynamic_cast<const Text*>(elements[i]);
        ASSERT_NE(text, nullptr);
        EXPECT_STREQ(text->getText().c_str(), char_cast(answer));
    };

    check_element(0, u8"Test");
    check_element(1, u8"测试");
    check_element(2, u8"テスト");
}

TEST(ControlLoadHandler, testRelativePath) {
    auto doc = loadTestDocument(GET_TESTFILE(u8"load/relativePaths.xopp"));
    ASSERT_TRUE(doc) << "Unable to load test file \"load/relativePaths.xopp\"";
    const auto& pdffile = doc->getPdfFilepath();

    auto check = [&pdffile](const fs::path& file, Util::PathStorageMode mode) {
        const auto doc = LoadHandler().loadDocument(file);
        ASSERT_TRUE(doc) << "Unable to load " << file.string();
        EXPECT_TRUE(fs::equivalent(doc->getPdfFilepath().lexically_normal(), pdffile.lexically_normal()))
                << "Paths \"" << doc->getPdfFilepath().string() << "\" and \"" << pdffile.string()
                << "\" are not equivalent";
        EXPECT_EQ(doc->getPathStorageMode(), mode);
    };

    auto saveReloadTest = [&](const fs::path& dir) {
        std::cout << "Test saving in " << dir.string() << std::endl;
        const fs::path outPath = dir / "xournalpp-test-units_ControlLoaderHandler_testRelativePath.xopp";
        ASSERT_TRUE(!fs::exists(outPath));

        SaveHandler saver;
        saver.prepareSave(doc.get(), outPath);
        saver.saveTo(outPath);
        EXPECT_TRUE(saver.getErrorMessage().empty());

        check(outPath, doc->getPathStorageMode());

        fs::remove(outPath);
    };

    doc->setPathStorageMode(Util::PathStorageMode::AS_ABSOLUTE_PATH);
    std::cout << "Mode PathStorageMode::AS_ABSOLUTE_PATH" << std::endl;

    saveReloadTest(fs::temp_directory_path());
    saveReloadTest(fs::current_path());

    doc->setPathStorageMode(Util::PathStorageMode::AS_RELATIVE_PATH);
    std::cout << "Mode PathStorageMode::AS_RELATIVE_PATH" << std::endl;

    saveReloadTest(fs::temp_directory_path());
    saveReloadTest(fs::current_path());
}
