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

#include <fstream>
#include <iostream>
#include <tuple>
#include <vector>

#include <config-test.h>
#include <gtest/gtest.h>

#include "gui/toolbarMenubar/model/ColorPalette.h"
#include "util/PathUtil.h"


std::string helper_read_file(fs::path path) {
    std::ifstream fs{path};
    std::stringstream stringStream{};
    stringStream << fs.rdbuf();
    return stringStream.str();
}

TEST(ColorPalette, testDefaultWrite) {
    Palette::create_default(GET_TESTFILE("palettes/default_tmp.gpl"));

    std::string createdDefault = helper_read_file(GET_TESTFILE("palettes/default_tmp.gpl"));
    std::string fixedDefault = helper_read_file(GET_TESTFILE("palettes/default.gpl"));

    EXPECT_EQ(fixedDefault, createdDefault);

    fs::remove(GET_TESTFILE("palettes/default_tmp.gpl"));
}

TEST(ColorPalette, testDefaultLoad) {
    Palette palette = Palette{GET_TESTFILE("palettes/default.gpl")};
    palette.load();

    Palette def = Palette{""};
    def.load_default();

    EXPECT_EQ((size_t)11, palette.size());
    EXPECT_EQ(palette.getColorAt(0).getName(), def.getColorAt(0).getName());
    EXPECT_EQ(palette.getColorAt(0).getIndex(), def.getColorAt(0).getIndex());
    EXPECT_EQ(palette.getColorAt(0).getColorU16().alpha, def.getColorAt(0).getColorU16().alpha);
    EXPECT_EQ(palette.getColorAt(0).getColorU16().red, def.getColorAt(0).getColorU16().red);
    EXPECT_EQ(palette.getColorAt(0).getColorU16().green, def.getColorAt(0).getColorU16().green);
    EXPECT_EQ(palette.getColorAt(0).getColorU16().blue, def.getColorAt(0).getColorU16().blue);
    EXPECT_EQ(palette.getColorAt(10).getName(), def.getColorAt(10).getName());
    EXPECT_EQ(palette.getColorAt(99).getName(), def.getColorAt(99).getName());
}

TEST(ColorPalette, testRainbowLoad) {
    Palette palette = Palette{GET_TESTFILE("palettes/rainbow.gpl")};
    palette.load();

    EXPECT_EQ((size_t)6, palette.size());
    EXPECT_EQ(palette.getColorAt(4).getName(), std::string{"Royal Blue"});
    EXPECT_EQ(palette.getColorAt(4).getIndex(), (size_t)4);
    EXPECT_EQ(palette.getColorAt(4).getColorU16().alpha, 0U);
    EXPECT_EQ(palette.getColorAt(4).getColorU16().red, 0U);
    EXPECT_EQ(palette.getColorAt(4).getColorU16().green, 0x4D4DU);
    EXPECT_EQ(palette.getColorAt(4).getColorU16().blue, 0xFFFFU);
    EXPECT_EQ(palette.getColorAt(4).getColor(), Color{0x004DFF});
}

TEST(ColorPalette, testNotExistLoad) {
    Palette palette = Palette{GET_TESTFILE("palettes/the_question_to_the_answer_42.gpl")};
    EXPECT_THROW(palette.load(), std::invalid_argument);
}

TEST(ColorPalette, testEmptyLoad) {
    Palette palette = Palette{GET_TESTFILE("palettes/empty.gpl")};
    EXPECT_THROW(palette.load(), std::invalid_argument);
}

TEST(ColorPalette, testWrongHeader) {
    Palette palette = Palette{GET_TESTFILE("palettes/wrong_header.gpl")};
    EXPECT_THROW(palette.load(), std::invalid_argument);
}

TEST(ColorPalette, testAttributesLoad) {
    Palette palette = Palette{GET_TESTFILE("palettes/broken_attribute.gpl")};
    EXPECT_THROW(palette.load(), std::invalid_argument);

    Palette palette2 = Palette{GET_TESTFILE("palettes/not_broken_attribute.gpl")};
    palette2.load();
    EXPECT_EQ(std::string{"Gray"}, palette2.getColorAt(0).getName());
}

TEST(ColorPalette, testWrongColor) {
    std::vector<std::tuple<fs::path, std::string>> path_exp{};
    path_exp.push_back(std::make_tuple(GET_TESTFILE("palettes/bad_color_1.gpl"),
                                       std::string{"RGB values bigger than 255 are not supported."}));
    path_exp.push_back(
            std::make_tuple(GET_TESTFILE("palettes/bad_color_2.gpl"), std::string{"The line 4 is malformed."}));
    path_exp.push_back(
            std::make_tuple(GET_TESTFILE("palettes/bad_color_3.gpl"), std::string{"The line 4 is malformed."}));


    for (auto pe: path_exp) {
        Palette palette = Palette{std::get<0>(pe)};
        try {
            palette.load();
            FAIL();
        } catch (const std::invalid_argument& e) {
            // check exception
            EXPECT_STREQ(std::get<1>(pe).c_str(), e.what());
        }
    }
}
