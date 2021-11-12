/**
 * @file ObjectIOStreamTest.cpp
 * @author Xournal++ Team
 * @brief
 * These tests test the serialization of different datatypes/classes.
 *
 * Note: Classes which implement there own serialization methods are not tested as part of this.
 * Such serialization methods are expected to be tested as part of these classes unit-tests.
 * This also helps to keep build times for `/util` unit-tests minimal.
 *
 * @license GNU GPLv2 or later
 *
 */
#include <array>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include <cairo.h>
#include <gtest/gtest.h>

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

