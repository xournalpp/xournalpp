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


TEST(LineStyle, testLineStyle)
{
    LineStyle ls;
    const double * data;
    int len = -42;

    EXPECT_EQ(ls.hasDashes(), false);
    EXPECT_EQ(ls.getDashes(data, len), false);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(len, 0);

    const double data2[] = {6, 2};
    ls.setDashes(data2, 2);
    EXPECT_EQ(ls.getDashes(data, len), true);
    EXPECT_EQ(len, 2);
    EXPECT_EQ(std::vector<double>(data, data + 2), std::vector<double>(data2, data2 + 2));
    EXPECT_EQ(ls.hasDashes(), true);
}
