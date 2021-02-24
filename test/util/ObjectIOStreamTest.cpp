#include <random>
#include <string>
#include <tuple>
#include <vector>

#include <cairo.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <util/serializing/BinObjectEncoding.h>
#include <util/serializing/HexObjectEncoding.h>
#include <util/serializing/ObjectInputStream.h>
#include <util/serializing/ObjectOutputStream.h>

using namespace std;

extern const char* XML_VERSION_STR;

class ObjectIOStreamTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ObjectIOStreamTest);
    CPPUNIT_TEST(testReadDouble);
    CPPUNIT_TEST(testReadInt);
    CPPUNIT_TEST(testReadSizeT);
    CPPUNIT_TEST(testReadString);
    CPPUNIT_TEST(testReadImage);
    CPPUNIT_TEST(testReadData);
    CPPUNIT_TEST(testReadComplexObject);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp() override {}
    void tearDown() override {}

    template <typename T, unsigned N>
    string serializeData(const array<T, N>& data) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeData(&data[0], N, sizeof(T));
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    string serializeImage(cairo_surface_t* surf) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeImage(surf);
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    string serializeString(string str) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeString(str);
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    string serializeSizeT(size_t x) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeSizeT(x);
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    string serializeDouble(double x) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeDouble(x);
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    string serializeInt(int x) {
        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeInt(x);
        auto outStr = outStream.getStr();
        return string(outStr->str, outStr->len);
    }

    void testReadComplexObject() {

        string objectName = "TestObject";
        double d = 42.;
        string s = "Test";
        int i = -1337;

        ObjectOutputStream outStream(new BinObjectEncoding);
        outStream.writeObject(objectName.c_str());
        outStream.writeDouble(d);
        outStream.writeString(s);
        outStream.writeInt(i);
        outStream.endObject();

        auto gstr = outStream.getStr();
        string str(gstr->str, gstr->len);

        ObjectInputStream stream;
        CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));

        string outputName = stream.readObject();
        CPPUNIT_ASSERT_EQUAL(outputName, objectName);
        double outputD = stream.readDouble();
        CPPUNIT_ASSERT_EQUAL(outputD, d);
        string outputS = stream.readString();
        CPPUNIT_ASSERT_EQUAL(outputS, s);
        int outputI = stream.readInt();
        CPPUNIT_ASSERT_EQUAL(outputI, i);
        stream.endObject();
    }

    template <typename T, unsigned int N>
    void testReadDataType(const array<T, N>& data) {
        string str = serializeData<T, N>(data);

        ObjectInputStream stream;
        CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));

        int length = 0;
        T* outputData = nullptr;
        stream.readData((void**)&outputData, &length);
        CPPUNIT_ASSERT_EQUAL(length, (int)N);

        for (size_t i = 0; i < (size_t)length / sizeof(T); ++i) {
            CPPUNIT_ASSERT_EQUAL(outputData[i], data.at(i));
        }
    }

    void testReadData() {
        testReadDataType<char, 3>({0, 42, -42});
        testReadDataType<long, 3>({0, 42, -42});
        testReadDataType<long long, 3>({0, 420000000000, -42000000000});
        testReadDataType<double, 3>({0, 42., -42.});
        testReadDataType<float, 3>({0, 42., -42.});
    }

    void testReadImage() {

        // Generate a "random" image and serialize/deserialize it.
        std::mt19937 gen(4242);
        std::uniform_int_distribution<unsigned char> distrib(0, 255);

        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 800, 800);
        unsigned char* surfaceData = cairo_image_surface_get_data(surface);

        int width = cairo_image_surface_get_width(surface);
        int height = cairo_image_surface_get_height(surface);

        for (unsigned i = 0; i < width * height * 4; ++i) {
            surfaceData[i] = distrib(gen);
        }

        string strSurface = serializeImage(surface);


        ObjectInputStream stream;
        CPPUNIT_ASSERT(stream.read(&strSurface[0], (int)strSurface.size() + 1));

        cairo_surface_t* outputSurface = stream.readImage();
        int widthOutput = cairo_image_surface_get_width(outputSurface);
        int heightOutput = cairo_image_surface_get_height(outputSurface);
        unsigned char* outputData = cairo_image_surface_get_data(surface);


        CPPUNIT_ASSERT_EQUAL(width, widthOutput);
        CPPUNIT_ASSERT_EQUAL(height, heightOutput);

        for (unsigned i = 0; i < width * height * 4; ++i) {
            CPPUNIT_ASSERT_EQUAL(surfaceData[i], outputData[i]);
        }

        cairo_surface_destroy(surface);
        cairo_surface_destroy(outputSurface);
    }

    void testReadString() {
        vector<string> intToTest = {
                "", "Hello World", XML_VERSION_STR, "1337",
                "Laborum beatae sit at. Tempore ex odio et non et iste et. Deleniti magni beatae quod praesentium dicta quas ducimus hic. Nemo vel est saepe voluptatibus. Sunt eveniet aut saepe consequatur fuga ad molestias.\n \
                Culpa nulla saepe alias magni nemo magni. Sed sit sint repellat doloremque. Quo ipsum debitis quos impedit. Omnis expedita veritatis nihil sint et itaque possimus. Nobis est fugit vel omnis. Dolores architecto laudantium nihil rerum."};

        vector<pair<string, string>> testData;
        for (string str: intToTest) {
            testData.push_back({serializeString(str), str});
        }

        for (auto data: testData) {
            string str = data.first;
            string x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            string output = stream.readString();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    void testReadSizeT() {
        vector<size_t> intToTest = {0, 1, 42, 1337, 10000000, 10000000000};

        vector<pair<string, size_t>> testData;
        for (size_t number: intToTest) {
            testData.push_back({serializeSizeT(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            size_t x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            size_t output = stream.readSizeT();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    void testReadInt() {
        vector<int> intToTest = {0, 1, -1, 42, -50000, -1337, 10000};

        vector<pair<string, int>> testData;
        for (int number: intToTest) {
            testData.push_back({serializeInt(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            int x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            int output = stream.readInt();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }


    void testReadDouble() {
        vector<double> doubleToTest = {0., 0.5, 42., 46.5, -85.2, -1337, 1e50};

        vector<pair<string, double>> testData;
        for (double number: doubleToTest) {
            testData.push_back({serializeDouble(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            double dbl = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            double output = stream.readDouble();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(dbl, output, 0.001);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectIOStreamTest);
