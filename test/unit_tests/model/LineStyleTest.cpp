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

TEST(LineStyle, testGetDashesScaledToStrokeWidth) {
    LineStyle ls;
    ls.setDashes(std::vector<double>({6, 2}));
    constexpr double eps = 1e-12;

    {
        auto scaled = ls.getDashesScaledToStrokeWidth(1);
        ASSERT_EQ(scaled.size(), 2);
        EXPECT_NEAR(scaled[0], 6.0, eps);
        EXPECT_NEAR(scaled[1], 2.0, eps);
    }

    {
        auto scaled = ls.getDashesScaledToStrokeWidth(2);
        ASSERT_EQ(scaled.size(), 2);
        EXPECT_NEAR(scaled[0], 12.0, eps);
        EXPECT_NEAR(scaled[1], 4.0, eps);
    }
}
