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

#include <gtest/gtest.h>

#include "control/ToolHandler.h"
#include "control/zoom/ZoomControl.h"

class DummyToolListener: public ToolListener {
public:
    void toolColorChanged() override {}
    void changeColorOfSelection() override {}
    void toolSizeChanged() override {}
    void toolFillChanged() override {}
    void toolLineStyleChanged() override {}
    void toolChanged() override {}
};

class FakeZoomControl: public ZoomControl {
public:
    double customZoomReal = 1.0;
    void setCustomZoomReal(double val) { customZoomReal = val; }
    double getZoomReal() const override { return customZoomReal; }
};

TEST(ToolHandlerZoomScalingTest, ThicknessWithoutZoomScaling) {
    DummyToolListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);
    ZoomControl zoom;
    zoom.setZoom100Value(1.0);
    zoom.setZoom(1.0);
    handler.setZoomControl(&zoom);

    handler.selectTool(TOOL_PEN);
    handler.setSize(TOOL_SIZE_MEDIUM);
    double baseThickness = handler.getThickness();

    zoom.setZoom(2.0);  // 200% zoom
    EXPECT_DOUBLE_EQ(handler.getThickness(), baseThickness);
}

TEST(ToolHandlerZoomScalingTest, ThicknessWithZoomScaling) {
    DummyToolListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);
    ZoomControl zoom;
    zoom.setZoom100Value(1.0);
    zoom.setZoom(1.0);
    handler.setZoomControl(&zoom);

    handler.selectTool(TOOL_PEN);
    handler.setSize(TOOL_SIZE_MEDIUM);
    double baseThickness = handler.getThickness();

    handler.getActiveTool()->setZoomScaling(true);

    // At 100% zoom (zoomReal = 1.0)
    EXPECT_NEAR(handler.getThickness(), baseThickness, 1e-5);

    // At 200% zoom (zoomReal = 2.0)
    zoom.setZoom(2.0);
    EXPECT_NEAR(handler.getThickness(), baseThickness / 2.0, 1e-5);

    // At 50% zoom (zoomReal = 0.5)
    zoom.setZoom(0.5);
    EXPECT_NEAR(handler.getThickness(), baseThickness / 0.5, 1e-5);
}

TEST(ToolHandlerZoomScalingTest, MinimumThicknessClamp) {
    DummyToolListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);
    FakeZoomControl zoom;
    handler.setZoomControl(&zoom);

    handler.selectTool(TOOL_PEN);
    handler.setSize(TOOL_SIZE_MEDIUM);

    handler.getActiveTool()->setZoomScaling(true);

    // High zoom: t / zoomReal falls below 0.01 (e.g. zoomReal = 700.0)
    zoom.setCustomZoomReal(700.0);
    EXPECT_DOUBLE_EQ(handler.getThickness(), 0.01);
}

TEST(ToolHandlerZoomScalingTest, NullZoomControlFallback) {
    DummyToolListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);
    handler.setZoomControl(nullptr);

    handler.selectTool(TOOL_PEN);
    handler.setSize(TOOL_SIZE_MEDIUM);
    double baseThickness = handler.getThickness();

    handler.getActiveTool()->setZoomScaling(true);
    EXPECT_DOUBLE_EQ(handler.getThickness(), baseThickness);
}

TEST(ToolHandlerZoomScalingTest, ZoomRealNearZeroFallback) {
    DummyToolListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);
    FakeZoomControl zoom;
    handler.setZoomControl(&zoom);

    handler.selectTool(TOOL_PEN);
    handler.setSize(TOOL_SIZE_MEDIUM);
    double baseThickness = handler.getThickness();

    handler.getActiveTool()->setZoomScaling(true);

    // Exactly at threshold 0.001
    zoom.setCustomZoomReal(0.001);
    EXPECT_DOUBLE_EQ(handler.getThickness(), baseThickness);

    // Below threshold 0.001
    zoom.setCustomZoomReal(0.0001);
    EXPECT_DOUBLE_EQ(handler.getThickness(), baseThickness);

    // Exactly zero
    zoom.setCustomZoomReal(0.0);
    EXPECT_DOUBLE_EQ(handler.getThickness(), baseThickness);
}
