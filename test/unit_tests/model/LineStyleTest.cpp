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

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/LineStyle.h"


TEST(LineStyle, testLineStyle) {
    LineStyle ls;

    EXPECT_EQ(ls.hasDashes(), false);
    EXPECT_EQ(ls.getDashes().empty(), true);

    const double data2[] = {6, 2};
    ls.setDashes(std::vector<double>(data2, data2 + 2));
    const auto& dashes = ls.getDashes();
    EXPECT_EQ(!dashes.empty(), true);
    EXPECT_EQ(dashes.size(), 2);
    EXPECT_EQ(dashes, std::vector<double>(data2, data2 + 2));
    EXPECT_EQ(ls.hasDashes(), true);
}
