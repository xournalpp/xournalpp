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
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp() {}
    void tearDown() {}

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
        CPPUNIT_ASSERT(stream.read(&strSurface[0], strSurface.size() + 1));

        cairo_surface_t* output_surface = stream.readImage();
        int width_output = cairo_image_surface_get_width(output_surface);
        int height_output = cairo_image_surface_get_height(output_surface);
        unsigned char* outputData = cairo_image_surface_get_data(surface);


        CPPUNIT_ASSERT_EQUAL(width, width_output);
        CPPUNIT_ASSERT_EQUAL(height, height_output);

        for (unsigned i = 0; i < width * height * 4; ++i) {
            CPPUNIT_ASSERT_EQUAL(surfaceData[i], outputData[i]);
        }

        cairo_surface_destroy(surface);
        cairo_surface_destroy(output_surface);
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
            CPPUNIT_ASSERT(stream.read(&str[0], str.size() + 1));
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
            CPPUNIT_ASSERT(stream.read(&str[0], str.size() + 1));
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
            CPPUNIT_ASSERT(stream.read(&str[0], str.size() + 1));
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
            CPPUNIT_ASSERT(stream.read(&str[0], str.size() + 1));
            double output = stream.readDouble();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(dbl, output, 0.001);
        }
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectIOStreamTest);
