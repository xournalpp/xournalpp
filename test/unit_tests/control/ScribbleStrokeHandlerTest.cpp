#include <algorithm>
#include <memory>
#include <vector>

#include <cairo.h>
#include <config-test.h>
#include <gtest/gtest.h>

#include "control/Control.h"
#include "control/ToolHandler.h"
#include "control/settings/Settings.h"
#include "control/tools/StrokeHandler.h"
#include "control/tools/StrokeStabilizerEnum.h"
#include "gui/GladeSearchpath.h"
#include "gui/inputdevices/PositionInputData.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageListener.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/InsertUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "util/Point.h"
#include "util/Range.h"
#include "util/Rectangle.h"
#include "view/Repaintable.h"
#include "view/overlays/OverlayView.h"

namespace {
// Keep the real document, input and history; only the window's action presentation is absent.
class HeadlessControl: public Control {
public:
    using Control::Control;
    void undoRedoChanged() override {}
};

class Buffer: public xoj::view::Repaintable {
public:
    std::unique_ptr<xoj::view::OverlayView> overlay;
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 400);
    Range finalizedRange;
    unsigned finalizations = 0;

    ~Buffer() override { cairo_surface_destroy(surface); }
    Range getVisiblePart() const override { return Range(0, 0, 400, 400); }
    double getZoom() const override { return 1; }
    ZoomControl* getZoomControl() const override { return nullptr; }
    double getWidth() const override { return 400; }
    double getHeight() const override { return 400; }
    xoj::util::Point<double> toWidgetCoordinates(const xoj::util::Point<double>& p) const override { return p; }
    xoj::util::Rectangle<double> toWidgetCoordinates(const xoj::util::Rectangle<double>& r) const override { return r; }
    void flagDirtyRegion(const Range&) const override {}
    void deleteOverlayView(xoj::view::OverlayView*, const Range&) override { overlay.reset(); }
    void drawAndDeleteToolView(xoj::view::ToolView* view, const Range& range) override {
        auto* cr = cairo_create(surface);
        view->drawWithoutDrawingAids(cr);
        cairo_destroy(cr);
        finalizedRange = range;
        ++finalizations;
        overlay.reset();
    }
    void preview() {
        auto* temporary = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 400);
        auto* cr = cairo_create(temporary);
        overlay->draw(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(temporary);
    }
    bool hasInk() const {
        cairo_surface_flush(surface);
        auto* data = cairo_image_surface_get_data(surface);
        const auto bytes = cairo_image_surface_get_stride(surface) * cairo_image_surface_get_height(surface);
        return std::any_of(data, data + bytes, [](unsigned char value) { return value != 0; });
    }
};

class ScribbleStrokeHandlerTest: public ::testing::Test {
protected:
    GladeSearchpath resources;
    std::unique_ptr<HeadlessControl> control;
    std::shared_ptr<XojPage> page;
    const std::vector<Point> points{{40, 40}, {120, 42}, {40, 44}, {120, 46}, {40, 48}, {120, 50}, {40, 52}};

    void SetUp() override {
        resources.addSearchDirectory(GET_RESOURCE_TEMPLATE_FOLDER);
        control = std::make_unique<HeadlessControl>(nullptr, &resources);
        auto* settings = control->getSettings();
        settings->setScribbleToEraseEnabled(true);
        settings->setScribbleSensitivity(50);
        settings->setScribbleMinimumPasses(4);
        settings->setEmptyLastPageAppend(EmptyLastPageAppendType::Disabled);
        settings->setStabilizerAveragingMethod(StrokeStabilizer::AveragingMethod::NONE);
        settings->setStabilizerPreprocessor(StrokeStabilizer::Preprocessor::NONE);
        auto* tools = control->getToolHandler();
        tools->selectTool(TOOL_PEN);
        tools->setDrawingType(DRAWING_TYPE_DEFAULT);
        page = std::make_shared<XojPage>(600, 800);
        control->getDocument()->addPage(page);
        page->setSelectedLayerId(1);
    }
    Layer* layer() const { return page->getSelectedLayer(); }
    Stroke* addTarget() {
        auto stroke = std::make_unique<Stroke>();
        stroke->setToolType(StrokeTool::PEN);
        stroke->setWidth(1);
        stroke->setColor(Colors::red);
        stroke->setFill(-1);
        stroke->setPointVector({{80, 20}, {80, 100}});
        auto* result = stroke.get();
        layer()->addElement(std::move(stroke));
        return result;
    }
    PositionInputData event(size_t i, double zoom = 1, double pressure = Point::NO_PRESSURE) const {
        PositionInputData result{};
        result.x = points[i].x * zoom;
        result.y = points[i].y * zoom;
        result.pressure = pressure;
        result.timestamp = static_cast<guint32>(i * 10);
        return result;
    }
    void draw(StrokeHandler& handler, Buffer& buffer, double zoom = 1, double pressure = Point::NO_PRESSURE) {
        handler.onButtonPressEvent(event(0, zoom, pressure), zoom);
        buffer.overlay = handler.createView(&buffer);
        for (size_t i = 1; i < points.size(); ++i) {
            EXPECT_TRUE(handler.onMotionNotifyEvent(event(i, zoom, pressure), zoom));
            buffer.preview();
        }
    }
};
}  // namespace

TEST_F(ScribbleStrokeHandlerTest, acceptsOnlyAtReleaseAndCancelsOverlayBeforeOneUndoAction) {
    control->getToolHandler()->getTool(TOOL_PEN).setFill(true);
    auto* target = addTarget();
    StrokeHandler handler(control.get(), page);
    Buffer buffer;
    draw(handler, buffer);
    EXPECT_EQ(layer()->getElements().size(), 1);
    EXPECT_FALSE(control->getUndoRedoHandler()->canUndo());
    handler.onButtonReleaseEvent(event(6), 1);
    EXPECT_TRUE(layer()->getElements().empty());
    EXPECT_EQ(buffer.finalizations, 1);
    EXPECT_FALSE(buffer.hasInk());
    EXPECT_FALSE(buffer.finalizedRange.empty());
    EXPECT_LE(buffer.finalizedRange.minY, 20);
    EXPECT_GE(buffer.finalizedRange.maxY, 100);
    auto* history = control->getUndoRedoHandler();
    history->undo();
    EXPECT_FALSE(history->canUndo());
    ASSERT_EQ(layer()->getElements().size(), 1);
    EXPECT_EQ(layer()->getElements().front().get(), target);
    history->redo();
    EXPECT_TRUE(layer()->getElements().empty());
}

TEST_F(ScribbleStrokeHandlerTest, disabledAndNoTargetFollowNormalInsertionAndOverlayFinalization) {
    for (bool enabled: {false, true}) {
        SCOPED_TRACE(enabled);
        control->getSettings()->setScribbleToEraseEnabled(enabled);
        if (!enabled) {
            addTarget();
        }
        const auto count = layer()->getElements().size();
        StrokeHandler handler(control.get(), page);
        Buffer buffer;
        draw(handler, buffer);
        handler.onButtonReleaseEvent(event(6), 1);
        EXPECT_EQ(layer()->getElements().size(), count + 1);
        EXPECT_TRUE(buffer.hasInk());
        EXPECT_TRUE(buffer.finalizedRange.empty());
        control->getUndoRedoHandler()->undo();
        EXPECT_EQ(layer()->getElements().size(), count);
        control->getUndoRedoHandler()->clearContents();
        layer()->getElements().clear();
    }
}

TEST_F(ScribbleStrokeHandlerTest, cancellationLeavesDocumentHistoryAndBackingBufferUntouched) {
    control->getToolHandler()->getTool(TOOL_PEN).setFill(true);
    addTarget();
    StrokeHandler handler(control.get(), page);
    Buffer buffer;
    draw(handler, buffer);
    handler.onSequenceCancelEvent();
    handler.onButtonReleaseEvent(event(6), 1);
    EXPECT_EQ(layer()->getElements().size(), 1);
    EXPECT_FALSE(control->getUndoRedoHandler()->canUndo());
    EXPECT_FALSE(buffer.hasInk());
    EXPECT_EQ(buffer.overlay, nullptr);
}

TEST_F(ScribbleStrokeHandlerTest, changedZoomPreferenceOrHistoryCannotErase) {
    for (int change = 0; change < 3; ++change) {
        SCOPED_TRACE(change);
        control->getSettings()->setScribbleToEraseEnabled(true);
        addTarget();
        StrokeHandler handler(control.get(), page);
        Buffer buffer;
        draw(handler, buffer);
        if (change == 0) {
            handler.onMotionNotifyEvent(event(6, 2), 2);
        } else if (change == 1) {
            control->getSettings()->setScribbleToEraseEnabled(false);
        } else {
            control->getUndoRedoHandler()->clearContents();
        }
        handler.onButtonReleaseEvent(event(6), 1);
        EXPECT_EQ(layer()->getElements().size(), 2);
        EXPECT_TRUE(buffer.hasInk());
        control->getUndoRedoHandler()->clearContents();
        layer()->getElements().clear();
    }
}

TEST_F(ScribbleStrokeHandlerTest, highlighterAndExplicitShapeToolsCannotEraseExistingInk) {
    for (bool highlighter: {true, false}) {
        auto* target = addTarget();
        auto* tools = control->getToolHandler();
        tools->selectTool(highlighter ? TOOL_HIGHLIGHTER : TOOL_PEN);
        tools->setDrawingType(highlighter ? DRAWING_TYPE_DEFAULT : DRAWING_TYPE_LINE);
        StrokeHandler handler(control.get(), page);
        Buffer buffer;
        draw(handler, buffer);
        handler.onButtonReleaseEvent(event(6), 1);
        ASSERT_GE(layer()->getElements().size(), 2);
        EXPECT_EQ(layer()->getElements().front().get(), target);
        control->getUndoRedoHandler()->clearContents();
        layer()->getElements().clear();
    }
}

TEST_F(ScribbleStrokeHandlerTest, pageCoordinatesAndPressureRemainEquivalentAtDifferentZooms) {
    for (double zoom: {0.25, 1.0, 4.0}) {
        for (double pressure: {Point::NO_PRESSURE, 0.5}) {
            SCOPED_TRACE(zoom);
            SCOPED_TRACE(pressure);
            addTarget();
            StrokeHandler handler(control.get(), page);
            Buffer buffer;
            draw(handler, buffer, zoom, pressure);
            handler.onButtonReleaseEvent(event(6, zoom, pressure), zoom);
            EXPECT_TRUE(layer()->getElements().empty());
            EXPECT_FALSE(buffer.hasInk());
            control->getUndoRedoHandler()->clearContents();
        }
    }
}

TEST_F(ScribbleStrokeHandlerTest, erasesBeforeShapeRecognitionWhenModeIsEnabled) {
    addTarget();
    control->getToolHandler()->setDrawingType(DRAWING_TYPE_SHAPE_RECOGNIZER);
    StrokeHandler handler(control.get(), page);
    Buffer buffer;
    draw(handler, buffer);
    handler.onButtonReleaseEvent(event(6), 1);
    EXPECT_TRUE(layer()->getElements().empty());
    auto* history = control->getUndoRedoHandler();
    ASSERT_TRUE(history->canUndo());
    history->undo();
    EXPECT_EQ(layer()->getElements().size(), 1);
    history->redo();
    EXPECT_TRUE(layer()->getElements().empty());
}

TEST_F(ScribbleStrokeHandlerTest, advancedOptionAndDrawingModeChangesInvalidateActiveGesture) {
    for (int change = 0; change < 3; ++change) {
        control->getSettings()->setScribbleSensitivity(50);
        control->getSettings()->setScribbleMinimumPasses(4);
        control->getToolHandler()->setDrawingType(DRAWING_TYPE_DEFAULT);
        auto* target = addTarget();
        StrokeHandler handler(control.get(), page);
        Buffer buffer;
        draw(handler, buffer);
        if (change == 0)
            control->getSettings()->setScribbleSensitivity(75);
        if (change == 1)
            control->getSettings()->setScribbleMinimumPasses(2);
        if (change == 2)
            control->getToolHandler()->setDrawingType(DRAWING_TYPE_SHAPE_RECOGNIZER);
        handler.onButtonReleaseEvent(event(6), 1);
        EXPECT_NE(layer()->indexOf(target), Element::InvalidIndex);
        EXPECT_GE(layer()->getElements().size(), 2);
        control->getUndoRedoHandler()->clearContents();
        layer()->getElements().clear();
    }
}

TEST_F(ScribbleStrokeHandlerTest, configuredShortGestureUsesRuntimeOptionsInBothModes) {
    for (auto mode: {DRAWING_TYPE_DEFAULT, DRAWING_TYPE_SHAPE_RECOGNIZER}) {
        for (int passes: {4, 2}) {
            control->getToolHandler()->setDrawingType(mode);
            control->getSettings()->setScribbleSensitivity(75);
            control->getSettings()->setScribbleMinimumPasses(passes);
            auto* target = addTarget();
            StrokeHandler handler(control.get(), page);
            Buffer buffer;
            handler.onButtonPressEvent(event(0), 1);
            buffer.overlay = handler.createView(&buffer);
            handler.onMotionNotifyEvent(event(1), 1);
            handler.onMotionNotifyEvent(event(2), 1);
            handler.onButtonReleaseEvent(event(2), 1);
            EXPECT_EQ(layer()->indexOf(target) == Element::InvalidIndex, passes == 2);
            control->getUndoRedoHandler()->clearContents();
            layer()->getElements().clear();
        }
    }
}
