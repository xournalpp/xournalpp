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
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"", u8"defaultpath", u8""), u8"");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{name}}%{name}", u8"x", u8""), u8"x}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{name}", u8"defaultpath.pdf", u8""), u8"defaultpath");

    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{file}", u8"pdfpath.pdf", u8"filepath.xopp"),
              u8"filepath");

    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{", u8"defaultpath", u8""), u8"%{");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{name%{name}}x", u8"", u8""), u8"}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"\\%\\{name%{name}}x", u8"", u8""), u8"\\%\\{name}x");
    EXPECT_EQ(SaveNameUtils::parseFilenameFromWildcardString(u8"%{name}", u8"      %{name}", u8""), u8"      %{name}");
}
