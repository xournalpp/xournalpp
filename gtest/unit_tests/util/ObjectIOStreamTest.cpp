#include <array>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include <cairo.h>
#include <gtest/gtest.h>

#include "model/Stroke.h"
#include "util/serializing/BinObjectEncoding.h"
#include "util/serializing/HexObjectEncoding.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"

extern const char* XML_VERSION_STR;


template <typename T, unsigned N>
std::string serializeData(const std::array<T, N>& data) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeData(&data[0], N, sizeof(T));
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeImage(cairo_surface_t* surf) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeImage(surf);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeString(const std::string& str) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeString(str);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeSizeT(size_t x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeSizeT(x);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeDouble(double x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeDouble(x);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeInt(int x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeInt(x);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

std::string serializeStroke(Stroke& stroke) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    stroke.serialize(outStream);
    auto outStr = outStream.getStr();
    return {outStr->str, outStr->len};
}

TEST(UtilObjectIOStream, testReadComplexObject) {

    std::string objectName = "TestObject";
    double d = 42.;
    std::string s = "Test";
    int i = -1337;

    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeObject(objectName.c_str());
    outStream.writeDouble(d);
    outStream.writeString(s);
    outStream.writeInt(i);
    outStream.endObject();

    auto gstr = outStream.getStr();
    std::string str(gstr->str, gstr->len);

    ObjectInputStream stream;
    EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));

    std::string outputName = stream.readObject();
    EXPECT_EQ(outputName, objectName);
    double outputD = stream.readDouble();
    EXPECT_EQ(outputD, d);
    std::string outputS = stream.readString();
    EXPECT_EQ(outputS, s);
    int outputI = stream.readInt();
    EXPECT_EQ(outputI, i);
    stream.endObject();
}

template <typename T, unsigned int N>
void testReadDataType(const std::array<T, N>& data) {
    std::string str = serializeData<T, N>(data);

    ObjectInputStream stream;
    EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));

    int length = 0;
    T* outputData = nullptr;
    stream.readData((void**)&outputData, &length);
    EXPECT_EQ(length, (int)N);

    for (size_t i = 0; i < (size_t)length / sizeof(T); ++i) { EXPECT_EQ(outputData[i], data.at(i)); }
}

TEST(UtilObjectIOStream, testReadData) {
    testReadDataType<char, 3>(std::array<char, 3>{0, 42, -42});
    testReadDataType<long, 3>(std::array<long, 3>{0, 42, -42});
    testReadDataType<long long, 3>(std::array<long long, 3>{0, 420000000000, -42000000000});
    testReadDataType<double, 3>(std::array<double, 3>{0, 42., -42.});
    testReadDataType<float, 3>(std::array<float, 3>{0, 42., -42.});
}

TEST(UtilObjectIOStream, testReadImage) {
    // Generate a "random" image and serialize/deserialize it.
    std::mt19937 gen(4242);
    std::uniform_int_distribution<unsigned char> distrib(0, 255);

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 800);
    unsigned char* surfaceData = cairo_image_surface_get_data(surface);

    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);

    for (unsigned i = 0; i < width * height * 4; ++i) { surfaceData[i] = distrib(gen); }

    std::string strSurface = serializeImage(surface);


    ObjectInputStream stream;
    EXPECT_TRUE(stream.read(&strSurface[0], (int)strSurface.size() + 1));

    cairo_surface_t* outputSurface = stream.readImage();
    int widthOutput = cairo_image_surface_get_width(outputSurface);
    int heightOutput = cairo_image_surface_get_height(outputSurface);
    unsigned char* outputData = cairo_image_surface_get_data(surface);


    EXPECT_EQ(width, widthOutput);
    EXPECT_EQ(height, heightOutput);

    for (unsigned i = 0; i < width * height * 4; ++i) { EXPECT_EQ(surfaceData[i], outputData[i]); }

    cairo_surface_destroy(surface);
    cairo_surface_destroy(outputSurface);
}

TEST(UtilObjectIOStream, testReadString) {
    std::vector<std::string> stringToTest{
            "", "Hello World", XML_VERSION_STR, "1337",
            "Laborum beatae sit at. Tempore ex odio et non et iste et. Deleniti magni beatae quod praesentium dicta quas ducimus hic. Nemo vel est saepe voluptatibus. Sunt eveniet aut saepe consequatur fuga ad molestias.\n \
                Culpa nulla saepe alias magni nemo magni. Sed sit sint repellat doloremque. Quo ipsum debitis quos impedit. Omnis expedita veritatis nihil sint et itaque possimus. Nobis est fugit vel omnis. Dolores architecto laudantium nihil rerum."};

    std::vector<std::pair<std::string, std::string>> testData;
    testData.reserve(stringToTest.size());
    for (auto&& str: stringToTest) { testData.emplace_back(serializeString(str), str); }

    for (auto&& data: testData) {
        std::string& str = data.first;
        std::string& x = data.second;

        ObjectInputStream stream;
        // The +1 stands for the \0 character
        EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));
        std::string output = stream.readString();
        EXPECT_EQ(x, output);
    }
}

TEST(UtilObjectIOStream, testReadSizeT) {
    std::vector<size_t> sizeTToTest{0, 1, 42, 1337, 10000000, 10000000000};

    std::vector<std::pair<std::string, size_t>> testData;
    testData.reserve(sizeTToTest.size());
    for (size_t number: sizeTToTest) { testData.emplace_back(serializeSizeT(number), number); }

    for (auto&& data: testData) {
        std::string& str = data.first;
        size_t x = data.second;

        ObjectInputStream stream;
        // The +1 stands for the \0 character
        EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));
        size_t output = stream.readSizeT();
        EXPECT_EQ(x, output);
    }
}

TEST(UtilObjectIOStream, testReadInt) {
    std::vector<int> intToTest{0, 1, -1, 42, -50000, -1337, 10000};

    std::vector<std::pair<std::string, int>> testData;
    testData.reserve(intToTest.size());
    for (auto&& number: intToTest) { testData.emplace_back(serializeInt(number), number); }

    for (auto&& data: testData) {
        std::string& str = data.first;
        int x = data.second;

        ObjectInputStream stream;
        // The +1 stands for the \0 character
        EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));
        int output = stream.readInt();
        EXPECT_EQ(x, output);
    }
}


TEST(UtilObjectIOStream, testReadDouble) {
    std::vector<double> doubleToTest{0., 0.5, 42., 46.5, -85.2, -1337, 1e50};

    std::vector<std::pair<std::string, double>> testData;
    testData.reserve(doubleToTest.size());
    for (auto&& number: doubleToTest) { testData.emplace_back(serializeDouble(number), number); }

    for (auto&& data: testData) {
        std::string& str = data.first;
        double dbl = data.second;

        ObjectInputStream stream;
        // The +1 stands for the \0 character
        EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));
        double output = stream.readDouble();
        EXPECT_DOUBLE_EQ(dbl, output);
    }
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
