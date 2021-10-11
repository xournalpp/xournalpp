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

#include <cppunit/extensions/HelperMacros.h>

#include "control/ToolEnums.h"


class ToolEnumsTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ToolEnumsTest);

    CPPUNIT_TEST(testToolSizeSerialization);
    CPPUNIT_TEST(testToolTypeSerialization);

    CPPUNIT_TEST_SUITE_END();

public:
    /**
     * Test whether the invariant
     *     fromString(toString(x)) == x
     * holds.
     */
    void testToolSizeSerialization() {
        for (unsigned int i = 0; i <= TOOL_SIZE_NONE; i++) {
            auto toolSize = static_cast<ToolSize>(i);
            std::string s = toolSizeToString(toolSize);
            CPPUNIT_ASSERT(s.empty() == false);
            CPPUNIT_ASSERT_EQUAL(toolSize, toolSizeFromString(s));
        }
    }

    /**
     * Test whether the invariant
     *     fromString(toString(x)) == x
     * holds.
     */
    void testToolTypeSerialization() {
        for (unsigned int i = 0; i < TOOL_END_ENTRY; i++) {
            auto toolType = static_cast<ToolType>(i);
            std::string s = toolTypeToString(toolType);
            CPPUNIT_ASSERT(s.empty() == false);
            CPPUNIT_ASSERT_EQUAL(toolType, toolTypeFromString(s));
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ToolEnumsTest);
