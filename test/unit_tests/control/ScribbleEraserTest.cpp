#include <cmath>
#include <limits>
#include <memory>
#include <mutex>
#include <vector>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/tools/ScribbleEraser.h"
#include "control/tools/ScribbleGestureRecognizer.h"
#include "control/xojfile/LoadHandler.h"
#include "model/Document.h"
#include "model/DocumentHandler.h"
#include "model/Image.h"
#include "model/Layer.h"
#include "model/Point.h"
#include "model/Stroke.h"
#include "model/Text.h"
#include "model/XojPage.h"
#include "undo/DeleteUndoAction.h"
#include "util/Range.h"

namespace {
using Path = std::vector<Point>;

class TestPage: public XojPage {
public:
    TestPage(): XojPage(600, 800) {}
    using XojPage::addLayer;
    using XojPage::setLayerVisible;
};

auto makeStroke(Path points, double width = 0.4) -> std::unique_ptr<Stroke> {
    auto stroke = std::make_unique<Stroke>();
    stroke->setWidth(width);
    stroke->setFill(-1);
    stroke->setColor(Colors::black);
    stroke->setToolType(StrokeTool::PEN);
    stroke->setPointVector(std::move(points));
    return stroke;
}

auto makeGesture() -> std::unique_ptr<Stroke> {
    return makeStroke({{0, 0}, {80, 2}, {0, 4}, {80, 6}, {0, 8}, {80, 10}, {0, 12}});
}

class ScribbleEraserTest: public ::testing::Test {
protected:
    DocumentHandler handler;
    Document document{&handler};
    std::shared_ptr<TestPage> page = std::make_shared<TestPage>();
    std::unique_ptr<Stroke> gesture = makeGesture();
    Range changed;

    void SetUp() override {
        document.addPage(page);
        page->setSelectedLayerId(1);
    }

    auto layer() -> Layer* { return page->getSelectedLayer(); }

    auto add(Path points, double width = 1.0) -> Stroke* {
        auto stroke = makeStroke(std::move(points), width);
        auto* result = stroke.get();
        layer()->addElement(std::move(stroke));
        return result;
    }

    auto erase() -> std::unique_ptr<DeleteUndoAction> {
        return ScribbleEraser::erase(document, page, *gesture, changed);
    }
};
}  // namespace

TEST_F(ScribbleEraserTest, deletesAllConfirmedStrokesInOneAction) {
    add({{20, -10}, {20, 30}});
    add({{40, -10}, {40, 30}});
    add({{60, -10}, {60, 30}});
    auto action = erase();
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->getClassName(), "DeleteUndoAction");
    EXPECT_TRUE(layer()->getElements().empty());
    EXPECT_FALSE(changed.empty());
    EXPECT_EQ(action->getPages().front(), page);
}

TEST_F(ScribbleEraserTest, preservesNoTargetAndBoundingBoxOnlyOverlap) {
    EXPECT_EQ(erase(), nullptr);
    auto* target = add({{10, 30}, {90, 30}, {90, -10}});
    ASSERT_TRUE(target->getBoundingBox().intersects(gesture->getBoundingBox()));
    EXPECT_EQ(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), target);
    EXPECT_TRUE(changed.empty());
}

TEST_F(ScribbleEraserTest, preservesAStrokeCrossedOnlyOnce) {
    auto* target = add({{40, -2}, {40, 1.25}}, 0.1);
    EXPECT_EQ(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), target);
}

TEST_F(ScribbleEraserTest, ignoresOtherLayersPagesAndNonStrokeElements) {
    auto text = std::make_unique<Text>();
    auto* textPointer = text.get();
    layer()->addElement(std::move(text));
    auto image = std::make_unique<Image>();
    auto* imagePointer = image.get();
    layer()->addElement(std::move(image));
    add({{40, -10}, {40, 30}});
    auto second = new Layer();
    auto otherStroke = makeStroke({{40, -10}, {40, 30}});
    auto* otherPointer = otherStroke.get();
    second->addElement(std::move(otherStroke));
    page->addLayer(second);
    page->setSelectedLayerId(1);
    auto otherPage = std::make_shared<XojPage>(600, 800);
    otherPage->getSelectedLayer()->addElement(makeStroke({{40, -10}, {40, 30}}));
    document.addPage(otherPage);
    ASSERT_NE(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 2);
    EXPECT_EQ(layer()->getElements()[0].get(), textPointer);
    EXPECT_EQ(layer()->getElements()[1].get(), imagePointer);
    EXPECT_EQ(second->getElements().front().get(), otherPointer);
    EXPECT_EQ(otherPage->getSelectedLayer()->getElements().size(), 1);
}

TEST_F(ScribbleEraserTest, rejectsBackgroundHiddenAndInvalidLayerSelections) {
    auto* target = add({{40, -10}, {40, 30}});
    for (size_t selection: {0U, 2U}) {
        page->setSelectedLayerId(selection);
        EXPECT_EQ(erase(), nullptr);
    }
    page->setSelectedLayerId(1);
    page->setLayerVisible(1, false);
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().front().get(), target);
    page->setLayerVisible(1, true);
    EXPECT_NE(erase(), nullptr);
}

TEST_F(ScribbleEraserTest, rejectsForeignPagesAndGesturesAlreadyInTheLayer) {
    add({{40, -10}, {40, 30}});
    auto foreignPage = std::make_shared<XojPage>(600, 800);
    EXPECT_EQ(ScribbleEraser::erase(document, foreignPage, *gesture, changed), nullptr);
    auto* original = gesture.get();
    layer()->addElement(std::move(gesture));
    EXPECT_EQ(ScribbleEraser::erase(document, page, *original, changed), nullptr);
    EXPECT_EQ(layer()->getElements().size(), 2);
}

TEST_F(ScribbleEraserTest, rejectsOtherInputStrokeToolsAndOrdinaryWriting) {
    add({{40, -10}, {40, 30}});
    for (auto tool: {StrokeTool::HIGHLIGHTER, StrokeTool::ERASER}) {
        gesture->setToolType(tool);
        EXPECT_EQ(erase(), nullptr);
    }
    gesture = makeStroke({{0, 0}, {80, 12}});
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().size(), 1);
}

TEST_F(ScribbleEraserTest, respectsTargetCapsAndActualWidths) {
    for (auto cap: {StrokeCapStyle::BUTT, StrokeCapStyle::ROUND, StrokeCapStyle::SQUARE}) {
        layer()->getElements().clear();
        auto* target = add({{40, 10}, {40, 30}}, 8);
        target->setStrokeCapStyle(cap);
        EXPECT_EQ(erase() != nullptr, cap != StrokeCapStyle::BUTT) << static_cast<int>(cap);
    }
}

TEST_F(ScribbleEraserTest, respectsGestureCapsAtIsolatedEndpointContacts) {
    for (auto cap: {StrokeCapStyle::BUTT, StrokeCapStyle::ROUND, StrokeCapStyle::SQUARE}) {
        layer()->getElements().clear();
        add({{-1, 0}, {-20, 0}, {-20, 12}, {-1, 12}}, 0.1);
        gesture->setWidth(4);
        gesture->setStrokeCapStyle(cap);
        EXPECT_EQ(erase() != nullptr, cap != StrokeCapStyle::BUTT) << static_cast<int>(cap);
    }
}

TEST_F(ScribbleEraserTest, usesPressureWidthsInsteadOfNominalWidthOrGlobalMaximum) {
    auto* thin = add({{200, -10, 20}, {82, -10, 0.2}, {82, 30, 0.2}}, 100);
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().front().get(), thin);
    layer()->getElements().clear();
    add({{81, -10, 6}, {81, 30, 6}}, 0.1);
    EXPECT_NE(erase(), nullptr);
}

TEST_F(ScribbleEraserTest, rejectsContactsInDashGapsAndSupportsVisibleDashes) {
    auto* target = add({{40, -10}, {40, 30}}, 0.2);
    LineStyle style;
    style.setDashes({1, 100});
    target->setLineStyle(style);
    EXPECT_EQ(erase(), nullptr);
    style.setDashes({100, 5});
    target->setLineStyle(style);
    EXPECT_NE(erase(), nullptr);
}

TEST_F(ScribbleEraserTest, supportsDotsAndDashedInputWithoutTreatingGapsAsInk) {
    auto* target = add({{40, -10}, {40, 30}}, 0.5);
    target->setStrokeCapStyle(StrokeCapStyle::ROUND);
    LineStyle dotted;
    dotted.setDashes({0, 1});
    target->setLineStyle(dotted);
    LineStyle dashed;
    dashed.setDashes({100, 1});
    gesture->setLineStyle(dashed);
    EXPECT_NE(erase(), nullptr);
}

TEST_F(ScribbleEraserTest, supportsFilledStrokes) {
    auto* target = add({{-10, -10}, {90, -10}, {90, 30}, {-10, 30}});
    target->setFill(255);
    EXPECT_NE(erase(), nullptr);
}

TEST_F(ScribbleEraserTest, invalidLaterTargetAbortsWithoutPartialDeletion) {
    auto* first = add({{40, -10}, {40, 30}});
    auto* invalid = add({{50, -10}, {50, 30}});
    invalid->setWidth(std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 2);
    EXPECT_EQ(layer()->getElements()[0].get(), first);
    EXPECT_EQ(layer()->getElements()[1].get(), invalid);
    EXPECT_TRUE(changed.empty());
}

TEST_F(ScribbleEraserTest, invalidPressureAndDashPatternsPreserveAllTargets) {
    auto* target = add({{40, -10, 0}, {40, 30, 0}});
    EXPECT_EQ(erase(), nullptr);
    for (double invalidPressure:
         {0.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}) {
        target->setPointVector(Path{{40, -10, 1}, {40, 30, invalidPressure}});
        EXPECT_EQ(erase(), nullptr);
    }
    target->setPointVector(Path{{40, -10}, {40, 30}});
    LineStyle invalid;
    invalid.setDashes({0, 0});
    target->setLineStyle(invalid);
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().size(), 1);
}

TEST_F(ScribbleEraserTest, exhaustedContactBudgetPreservesAllTargets) {
    auto* first = add({{40, -10}, {40, 30}});
    Path missed;
    for (size_t i = 0; i < 1000; ++i) {
        missed.emplace_back(10, 30);
        missed.emplace_back(90, 30);
        missed.emplace_back(90, -10);
        missed.emplace_back(90, 30);
    }
    for (int i = 0; i < 100; ++i) {
        add(missed);
    }
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().size(), 101);
    EXPECT_EQ(layer()->getElements().front().get(), first);
    EXPECT_TRUE(changed.empty());
}

TEST_F(ScribbleEraserTest, preservesContinuousContactAlongOnlyOneOutwardPass) {
    Path path;
    for (size_t i = 0; i <= 80; ++i) {
        path.emplace_back(static_cast<double>(i), static_cast<double>(i) / 40);
    }
    for (const Point& point: Path{{0, 4}, {80, 6}, {0, 8}, {80, 10}, {0, 12}}) {
        path.push_back(point);
    }
    gesture = makeStroke(path, 0.1);
    const auto* target = add({{20, 0.5}, {60, 1.5}}, 0.1);
    EXPECT_EQ(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), target);
}

TEST_F(ScribbleEraserTest, recognizesRepeatedRecordedHitsAtDifferentPositionsOfTheSameStroke) {
    auto recorded = LoadHandler{}.loadDocument(GET_TESTFILE(u8"scribble/uneven-scrubs.xopp"));
    ASSERT_NE(recorded, nullptr);
    const auto& saved = recorded->getPage(2)->getSelectedLayer()->getElements();
    const auto& scrub = static_cast<const Stroke&>(*saved.back());
    gesture = makeStroke(scrub.getPointVector(), 1.41);
    add({{220, 150}, {220, 250}}, 2);
    ASSERT_TRUE(ScribbleGestureRecognizer::isCandidate(gesture->getPointVector()));
    ASSERT_EQ(gesture->getPointVector().back().z, Point::NO_PRESSURE);
    EXPECT_NE(erase(), nullptr);
    EXPECT_TRUE(layer()->getElements().empty());
}

TEST_F(ScribbleEraserTest, acceptsUnusedFinalPressureSentinelOnLoadedTargetsAndGestures) {
    for (bool pressureGesture: {false, true}) {
        SCOPED_TRACE(pressureGesture);
        gesture = makeGesture();
        if (pressureGesture) {
            auto path = gesture->getPointVector();
            for (auto& point: path) {
                point.z = 0.4;
            }
            path.back().z = Point::NO_PRESSURE;
            gesture->setPointVector(path);
        }
        add({{40, -10, 1}, {40, 30}});
        ASSERT_NE(erase(), nullptr);
        EXPECT_TRUE(layer()->getElements().empty());
    }
}

TEST_F(ScribbleEraserTest, preservesSmallTransverseJitterDuringOnlyOneTargetVisit) {
    Path path;
    for (size_t i = 0; i <= 80; ++i) {
        const double x = static_cast<double>(i);
        path.emplace_back(x, 0.06 * std::sin(0.4 * x));
    }
    for (const Point& point: Path{{0, 4}, {80, 6}, {0, 8}, {80, 10}, {0, 12}}) {
        path.push_back(point);
    }
    gesture = makeStroke(path, 0.1);
    const auto* target = add({{20, 0}, {60, 0}}, 0.1);
    EXPECT_EQ(erase(), nullptr);
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), target);
}

TEST_F(ScribbleEraserTest, erasesSeparatedTargetsWithAMovingScrub) {
    Path path;
    for (size_t i = 0; i <= 12; ++i) {
        path.emplace_back(10.0 * static_cast<double>(i), i % 2 == 0 ? 0.0 : 40.0);
    }
    gesture = makeStroke(path);
    for (double x: {15, 35, 55, 75, 95, 115}) {
        add({{x - 10, 20}, {x + 10, 20}}, 1);
    }
    auto* untouched = add({{160, 0}, {160, 40}});
    auto action = erase();
    ASSERT_NE(action, nullptr);
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), untouched);
}

TEST_F(ScribbleEraserTest, configuredShortGestureErasesOnlyWithReducedPassRequirement) {
    gesture = makeStroke({{0, 0}, {80, 2}, {0, 4}});
    auto* target = add({{40, -10}, {40, 30}});
    EXPECT_EQ(erase(), nullptr);
    EXPECT_EQ(layer()->getElements().front().get(), target);
    EXPECT_NE(ScribbleEraser::erase(document, page, *gesture, changed, {75, 2}), nullptr);
    EXPECT_TRUE(layer()->getElements().empty());
}

TEST_F(ScribbleEraserTest, confirmsReturnAtLaterCrossingOfClosedShapeInTheSameSegment) {
    Path path;
    for (int i = 0; i <= 12; ++i) path.emplace_back(100.0 + 10 * i, i % 2 ? 240.0 : 180.0);
    gesture = makeStroke(path);
    add({{205, 190}, {225, 190}, {225, 230}, {205, 230}, {205, 190}});
    EXPECT_NE(erase(), nullptr);
    EXPECT_TRUE(layer()->getElements().empty());
}
