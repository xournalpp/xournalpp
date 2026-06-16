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

#include "control/xojfile/LoadHandler.h"
#include "model/LineStyle.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "util/serializing/BinObjectEncoding.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"


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
    constexpr double tol = 1e-8;

    {
        auto scaled = ls.getDashesScaledToStrokeWidth(1);
        ASSERT_EQ(scaled.size(), 2);
        EXPECT_NEAR(scaled[0], 6.0, tol);
        EXPECT_NEAR(scaled[1], 2.0, tol);
    }

    {
        auto scaled = ls.getDashesScaledToStrokeWidth(2);
        ASSERT_EQ(scaled.size(), 2);
        EXPECT_NEAR(scaled[0], 12.0, tol);
        EXPECT_NEAR(scaled[1], 4.0, tol);
    }
}

TEST(LineStyle, testScaleDashesToStrokeWidth) {
    LineStyle ls;
    constexpr double tol = 1e-8;

    ls.setDashes(std::vector<double>({6, 2}));
    ls.scaleDashesToStrokeWidth(1);
    auto scaled = ls.getDashes();
    ASSERT_EQ(scaled.size(), 2);
    EXPECT_NEAR(scaled[0], 6.0, tol);
    EXPECT_NEAR(scaled[1], 2.0, tol);

    ls.setDashes(std::vector<double>({6, 2}));
    ls.scaleDashesToStrokeWidth(3);
    scaled = ls.getDashes();
    ASSERT_EQ(scaled.size(), 2);
    EXPECT_NEAR(scaled[0], 18.0, tol);
    EXPECT_NEAR(scaled[1], 6.0, tol);
}

TEST(LineStyle, testSetScaleDashes) {
    {
        LineStyle ls;
        ls.setScaleDashes();
        ASSERT_TRUE(ls.scaleDashes());
    }

    {
        LineStyle ls;
        ls.setScaleDashes("scaled_dash");
        ASSERT_TRUE(ls.scaleDashes());
    }

    {
        LineStyle ls;
        ls.setScaleDashes("dot");
        ASSERT_FALSE(ls.scaleDashes());
    }

    {
        LineStyle ls;
        ls.setScaleDashes("dot_scaled");
        ASSERT_FALSE(ls.scaleDashes());
    }
}

TEST(LineStyle, testCopyPasteMantainsLineStyle) {
    constexpr std::array<size_t, 2> strokeIndexes = {0,
                                                     5};  // One pressure sensitive and one pressure insensitive stroke

    LoadHandler loadHandler;
    auto doc = loadHandler.loadDocument(GET_TESTFILE(u8"load/scaled-dashes.xopp"));
    ASSERT_NE(doc.get(), nullptr);

    auto elems = doc->getPage(0)->getLayersView()[0]->getElementsView();
    ASSERT_GT(elems.size(), 5);

    for (size_t i: strokeIndexes) {
        auto s = dynamic_cast<const Stroke*>(elems[i]);
        ASSERT_NE(s, nullptr);

        ObjectOutputStream out(new BinObjectEncoding());
        s->serialize(out);

        GString* clipboardData = out.stealData();
        ASSERT_NE(clipboardData, nullptr);

        ObjectInputStream in;
        ASSERT_TRUE(in.read(clipboardData->str, clipboardData->len));

        Stroke pastedStroke;
        pastedStroke.readSerialized(in);

        const LineStyle& originalStyle = s->getLineStyle();
        const LineStyle& pastedStyle = pastedStroke.getLineStyle();

        EXPECT_EQ(originalStyle, pastedStyle);

        g_string_free(clipboardData, TRUE);
    }
}

TEST(LineStyle, testStrokeScalingOnLineStyle) {
    constexpr std::array<size_t, 2> strokeIndexes = {2,
                                                     3};  // One pressure sensitive and one pressure insensitive stroke
    constexpr double tol = 1e-8;
    constexpr double factor = 5.0;

    auto collinearVectors = [](const std::vector<double>& v1, const std::vector<double>& v2) -> bool {
        if (v1.size() == 0)
            return false;
        if (v1.size() != v2.size())
            return false;

        size_t i = 0;
        while (i < v1.size() && std::abs(v1[i]) < tol) ++i;

        if (i == v1.size())
            return true;

        double k = v2[i] / v1[i];

        for (size_t j = 0; j < v1.size(); ++j) {
            if (std::abs(v2[j] - k * v1[j]) > tol)
                return false;
        }

        return true;
    };

    LoadHandler loadHandler;
    auto doc = loadHandler.loadDocument(GET_TESTFILE(u8"load/scaled-dashes.xopp"));
    ASSERT_NE(doc.get(), nullptr);

    auto elems = doc->getPage(0)->getLayersView()[0]->getElementsView();
    ASSERT_GT(elems.size(), 0);

    bool pressureSensitive = true;
    for (size_t i: strokeIndexes) {
        auto baseStroke = dynamic_cast<const Stroke*>(elems[i]);
        ASSERT_NE(baseStroke, nullptr);

        const LineStyle originalStyle = baseStroke->getLineStyle();
        const double originalWidth = baseStroke->getWidth();

        std::unique_ptr<Stroke> s(dynamic_cast<Stroke*>(baseStroke->clone().release()));
        ASSERT_NE(s, nullptr);

        s->scale(0.0, 0.0, factor, factor, 0.0, false);

        const LineStyle& scaledStyle = s->getLineStyle();

        EXPECT_GT(s->getWidth(), originalWidth);

        EXPECT_TRUE(originalStyle.hasDashes());

        if (pressureSensitive) {
            EXPECT_TRUE(originalStyle.scaleDashes());
            EXPECT_EQ(scaledStyle, originalStyle);
            pressureSensitive = false;
        } else {
            const auto& originalDashes = originalStyle.getDashes();
            auto scaledDashes = scaledStyle.getDashes();
            EXPECT_TRUE(collinearVectors(originalDashes, scaledDashes));
            EXPECT_NEAR(originalDashes[0] * factor, scaledDashes[0], tol);
        }
    }
}
