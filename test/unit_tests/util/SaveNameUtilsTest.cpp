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

#include "util/SaveNameUtils.h"

TEST(SaveNameUtils, testWildcardExpansion) {
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("", "defaultpath"), "");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("%{name}}%{name}", "x"), "x}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("%{name}", "defaultpath.pdf"), "defaultpath");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("%{", "defaultpath"), "%{");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("%{name%{name}}x", ""), "}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("\\%\\{name%{name}}x", ""), "\\%\\{name}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString("%{name}", "      %{name}"), "      %{name}");
}
