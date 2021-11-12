/**
 * @file StrokeTest.cpp
 * @author Xournal++ Team
 * @license GNU GPLv2 or later
 * @brief
 * These tests test the Stroke Class
 *
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "model/Stroke.h"
#include "util/serializing/BinObjectEncoding.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"

std::string serializeStroke(Stroke& stroke) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    stroke.serialize(outStream);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}


void assertStrokeEquality(const Stroke& stroke1, const Stroke& stroke2) {
    EXPECT_EQ(stroke1.getAudioFilename(), stroke2.getAudioFilename());
    EXPECT_EQ(stroke1.getToolType(), stroke2.getToolType());
    EXPECT_EQ(stroke1.getFill(), stroke2.getFill());
    EXPECT_EQ(stroke1.getWidth(), stroke2.getWidth());

    double avgPressure1 = stroke1.getAvgPressure();
    double avgPressure2 = stroke2.getAvgPressure();

    if (!std::isnan(avgPressure1)) {
        EXPECT_DOUBLE_EQ(avgPressure1, avgPressure2);
    } else {
        EXPECT_TRUE(std::isnan(avgPressure2));
    }

    std::vector<Point> points1 = stroke1.getPointVector();
    std::vector<Point> points2 = stroke2.getPointVector();

    EXPECT_EQ(points1.size(), points2.size());
    for (size_t i = 0; i < points1.size(); ++i) { EXPECT_TRUE(points1[i].equalsPos(points2[i])); }
}

TEST(UtilObjectIOStream, testReadStroke) {
    std::vector<Stroke> strokes(8);
    // strokes[0]: empty stroke

    strokes[1].addPoint(Point(42, 42));
    strokes[1].addPoint(Point(42.1, 42.1));
    strokes[1].addPoint(Point(1312., 8));

    strokes[2].setWidth(42.);

    strokes[3].setFill(245);

    strokes[4].setToolType(StrokeTool::STROKE_TOOL_ERASER);

    strokes[5].setAudioFilename("foo.mp3");

    // strokes[6]: complex stroke
    strokes[6].addPoint(Point(-1312., 8));
    strokes[6].addPoint(Point(-42, -42));
    strokes[6].addPoint(Point(42.1, -42.1));
    strokes[6].setPressure({42., 1332.});
    strokes[6].setWidth(1337.);
    strokes[6].setFill(-1);
    strokes[6].setToolType(StrokeTool::STROKE_TOOL_PEN);
    strokes[6].setAudioFilename("assets/bar.mp3");

    // strokes[7]: invalid stroke
    strokes[7].addPoint(Point(0., 0.));
    strokes[7].addPoint(Point(1., 2.));
    strokes[7].addPoint(Point(1., 2.));
    strokes[7].setPressure({42., 1332.});
    strokes[7].setFill(-42);
    strokes[7].setToolType((StrokeTool)42);
    strokes[7].setWidth(-1337.);


    for (auto&& stroke: strokes) {
        std::string out_string = serializeStroke(stroke);
        ObjectInputStream istream;
        istream.read(out_string.c_str(), (int)out_string.size());

        Stroke in_stroke;
        in_stroke.readSerialized(istream);
        assertStrokeEquality(stroke, in_stroke);
    }
}
