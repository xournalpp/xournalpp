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

#include "control/Tool.h"
#include "control/ToolBase.h"

TEST(ToolZoomScalingTest, DefaultIsDisabled) {
    ToolBase toolBase;
    EXPECT_FALSE(toolBase.isZoomScaling());
}

TEST(ToolZoomScalingTest, ToggleZoomScaling) {
    ToolBase toolBase;
    toolBase.setZoomScaling(true);
    EXPECT_TRUE(toolBase.isZoomScaling());
    toolBase.setZoomScaling(false);
    EXPECT_FALSE(toolBase.isZoomScaling());
}

TEST(ToolZoomScalingTest, ToolCopyPreservesZoomScaling) {
    std::array<double, Tool::toolSizes> thicknesses = {0.5, 1.0, 2.0, 3.0, 5.0};
    Tool tool1("pen", TOOL_PEN, Colors::black, thicknesses);
    tool1.setZoomScaling(true);

    Tool tool2(tool1);
    EXPECT_TRUE(tool2.isZoomScaling());
}
