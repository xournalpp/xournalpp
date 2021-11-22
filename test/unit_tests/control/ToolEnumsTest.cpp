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

#include <string>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/ToolEnums.h"

/**
 * Test whether the invariant
 *     fromString(toString(x)) == x
 * holds.
 */
TEST(ToolEnumsTest, testToolSizeSerialization) {
    for (unsigned int i = 0; i <= TOOL_SIZE_NONE; i++) {
        auto toolSize = static_cast<ToolSize>(i);
        std::string s = toolSizeToString(toolSize);
        GTEST_ASSERT_FALSE(s.empty());
        GTEST_ASSERT_EQ(toolSize, toolSizeFromString(s));
    }
}

/**
 * Test whether the invariant
 *     fromString(toString(x)) == x
 * holds.
 */
TEST(ToolEnumsTest, testToolTypeSerialization) {
    for (unsigned int i = 0; i < TOOL_END_ENTRY; i++) {
        auto toolType = static_cast<ToolType>(i);
        std::string s = toolTypeToString(toolType);
        GTEST_ASSERT_FALSE(s.empty());
        GTEST_ASSERT_EQ(toolType, toolTypeFromString(s));
    }
}
