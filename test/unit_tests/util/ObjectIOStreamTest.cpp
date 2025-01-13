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
#include "util/serializing/Serializable.h"

template <typename T>
std::string serializeDataVector(const std::vector<T>& data) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeData(data);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeImage(cairo_surface_t* surf) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    std::string data{reinterpret_cast<char*>(cairo_image_surface_get_data(surf))};
    outStream.writeImage(data);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeString(const std::string& str) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeString(str);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeSizeT(size_t x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeSizeT(x);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeDouble(double x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeDouble(x);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeInt(int x) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    outStream.writeInt(x);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

std::string serializeStroke(Stroke& stroke) {
    ObjectOutputStream outStream(new BinObjectEncoding);
    stroke.serialize(outStream);
    auto outStr = outStream.getStr();
    auto resStr = std::string{outStr->str, outStr->len};
    g_string_free(outStr, true);
    return resStr;
}

template <typename T>
void testReadDataType(const std::vector<T>& data) {
    std::string str = serializeDataVector<T>(data);
    ObjectInputStream stream;
    EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));

    std::vector<T> outputData;
    stream.readData(outputData);

    EXPECT_EQ(data, outputData);
}


TEST(UtilObjectIOStream, testReadData) {
    testReadDataType(std::vector<char>{0, 42, -42});
    testReadDataType(std::vector<long>{0, 42, -42});
    testReadDataType(std::vector<long long>{0, 420000000000, -42000000000});
    testReadDataType(std::vector<float>{0, 42., -42.});
    testReadDataType(std::vector<double>{0, 42., -42.});

    struct Data {
        bool operator==(const Data& o) const { return s == o.s && f == o.f && b == o.b; }
        size_t s;
        float f;
        bool b;
    };
    testReadDataType(std::vector<Data>{{243254, 0.4534314213f, true}, {2, -4243213.32f, false}});
}

TEST(UtilObjectIOStream, testReadImage) {
    // Generate a "random" image and serialize/deserialize it.
    std::mt19937 gen(4242);
    // MSVC does not support uniform_int_distribution<unsigned char> so we use a longer int
    std::uniform_int_distribution<unsigned int> distrib(0, 255);

    const cairo_format_t format = CAIRO_FORMAT_ARGB32;
    cairo_surface_t* surface = cairo_image_surface_create(format, 800, 800);
    unsigned char* surfaceData = cairo_image_surface_get_data(surface);

    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);

    for (int i = 0; i < width * height * 4; ++i) { surfaceData[i] = static_cast<unsigned char>(distrib(gen)); }

    std::string strSurface = serializeImage(surface);

    ObjectInputStream stream;
    EXPECT_TRUE(stream.read(&strSurface[0], (int)strSurface.size() + 1));

    std::string outputStr = stream.readImage();

    cairo_surface_t* outputSurface =
            cairo_image_surface_create_for_data(reinterpret_cast<unsigned char*>(outputStr.data()), format, width,
                                                height, cairo_format_stride_for_width(format, width));
    EXPECT_NE(outputSurface, nullptr);

    int widthOutput = cairo_image_surface_get_width(outputSurface);
    int heightOutput = cairo_image_surface_get_height(outputSurface);
    unsigned char* outputData = cairo_image_surface_get_data(surface);

    EXPECT_EQ(width, widthOutput);
    EXPECT_EQ(height, heightOutput);

    for (int i = 0; i < width * height * 4; ++i) { EXPECT_EQ(surfaceData[i], outputData[i]); }

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

TEST(UtilObjectIOStream, testReadComplexObject) {
    std::string objectName = "TestObject";
    std::vector<std::string> subobjectNames = {"FirstTestSubobject", "SecondTestSubobject"};
    double d = 42.;
    std::string s = "Test";
    int i = -1337;
    size_t n = 1234567;

    try {
        for (size_t iterNum = 0; iterNum < subobjectNames.size(); ++iterNum) {
            ObjectOutputStream outStream(new BinObjectEncoding);
            outStream.writeObject(objectName.c_str());
            outStream.writeDouble(d);
            outStream.writeString(s);

            outStream.writeObject(subobjectNames[iterNum].c_str());
            if (iterNum == 0) {
                outStream.writeInt(i);
            } else {
                outStream.writeSizeT(n);
                outStream.writeSizeT(12 * n);
            }
            outStream.endObject();

            outStream.writeDouble(-d);
            outStream.endObject();

            auto gstr = outStream.getStr();
            std::string str(gstr->str, gstr->len);
            g_string_free(gstr, true);

            ObjectInputStream stream;
            EXPECT_TRUE(stream.read(&str[0], (int)str.size() + 1));

            std::string outputName = stream.readObject();
            EXPECT_EQ(outputName, objectName);
            double outputD = stream.readDouble();
            EXPECT_EQ(outputD, d);
            std::string outputS = stream.readString();
            EXPECT_EQ(outputS, s);

            std::string nextsubname = stream.getNextObjectName();
            EXPECT_EQ(nextsubname, subobjectNames[iterNum]);
            std::string subname = stream.readObject();
            EXPECT_EQ(subname, subobjectNames[iterNum]);
            if (iterNum == 0) {
                int outputI = stream.readInt();
                EXPECT_EQ(outputI, i);
            } else {
                size_t outputN = stream.readSizeT();
                EXPECT_EQ(outputN, n);
                size_t output12N = stream.readSizeT();
                EXPECT_EQ(output12N, 12 * n);
            }
            stream.endObject();

            double outputMinusD = stream.readDouble();
            EXPECT_EQ(outputMinusD, -d);

            stream.endObject();
        }
    } catch (const InputStreamException& e) {
        std::cerr << "InputStreamException testing complex object: " << e.what() << std::endl;
        FAIL();
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

    strokes[4].setToolType(StrokeTool::ERASER);

    strokes[5].setAudioFilename("foo.mp3");

    // strokes[6]: complex stroke
    strokes[6].addPoint(Point(-1312., 8));
    strokes[6].addPoint(Point(-42, -42));
    strokes[6].addPoint(Point(42.1, -42.1));
    strokes[6].setPressure({42., 1332.});
    strokes[6].setWidth(1337.);
    strokes[6].setFill(-1);
    strokes[6].setToolType(StrokeTool::PEN);
    strokes[6].setAudioFilename("assets/bar.mp3");

    // strokes[7]: invalid stroke
    strokes[7].addPoint(Point(0., 0.));
    strokes[7].addPoint(Point(1., 2.));
    strokes[7].addPoint(Point(1., 2.));
    strokes[7].setPressure({42., 1332.});
    strokes[7].setFill(-42);
    strokes[7].setToolType(static_cast<StrokeTool::Value>(42));
    strokes[7].setWidth(-1337.);

    size_t i = 0;
    try {
        for (auto&& stroke: strokes) {
            std::string out_string = serializeStroke(stroke);
            ObjectInputStream istream;
            istream.read(out_string.c_str(), (int)out_string.size());

            Stroke in_stroke;
            in_stroke.readSerialized(istream);
            assertStrokeEquality(stroke, in_stroke);
            ++i;
        }
    } catch (const InputStreamException& e) {
        std::cerr << "InputStreamException testing stroke " << i << ": " << e.what() << std::endl;
        FAIL();
    }
}
