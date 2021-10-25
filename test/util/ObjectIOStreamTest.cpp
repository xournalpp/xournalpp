#include <array>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include <cairo.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <model/Stroke.h>
#include <util/serializing/BinObjectEncoding.h>
#include <util/serializing/HexObjectEncoding.h>
#include <util/serializing/ObjectInputStream.h>
#include <util/serializing/ObjectOutputStream.h>

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
    CPPUNIT_TEST(testReadStroke);
    CPPUNIT_TEST_SUITE_END();


public:
    void setUp() override {}
    void tearDown() override {}

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

    void testReadComplexObject() {

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
        CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));

        std::string outputName = stream.readObject();
        CPPUNIT_ASSERT_EQUAL(outputName, objectName);
        double outputD = stream.readDouble();
        CPPUNIT_ASSERT_EQUAL(outputD, d);
        std::string outputS = stream.readString();
        CPPUNIT_ASSERT_EQUAL(outputS, s);
        int outputI = stream.readInt();
        CPPUNIT_ASSERT_EQUAL(outputI, i);
        stream.endObject();
    }

    template <typename T, unsigned int N>
    void testReadDataType(const std::array<T, N>& data) {
        std::string str = serializeData<T, N>(data);

        ObjectInputStream stream;
        CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));

        int length = 0;
        T* outputData = nullptr;
        stream.readData((void**)&outputData, &length);
        CPPUNIT_ASSERT_EQUAL(length, (int)N);

        for (size_t i = 0; i < (size_t)length / sizeof(T); ++i) { CPPUNIT_ASSERT_EQUAL(outputData[i], data.at(i)); }
    }

    void testReadData() {
        testReadDataType<char, 3>(std::array<char, 3>{0, 42, -42});
        testReadDataType<long, 3>(std::array<long, 3>{0, 42, -42});
        testReadDataType<long long, 3>(std::array<long long, 3>{0, 420000000000, -42000000000});
        testReadDataType<double, 3>(std::array<double, 3>{0, 42., -42.});
        testReadDataType<float, 3>(std::array<float, 3>{0, 42., -42.});
    }

    void testReadImage() {

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
        CPPUNIT_ASSERT(stream.read(&strSurface[0], (int)strSurface.size() + 1));

        cairo_surface_t* outputSurface = stream.readImage();
        int widthOutput = cairo_image_surface_get_width(outputSurface);
        int heightOutput = cairo_image_surface_get_height(outputSurface);
        unsigned char* outputData = cairo_image_surface_get_data(surface);


        CPPUNIT_ASSERT_EQUAL(width, widthOutput);
        CPPUNIT_ASSERT_EQUAL(height, heightOutput);

        for (unsigned i = 0; i < width * height * 4; ++i) { CPPUNIT_ASSERT_EQUAL(surfaceData[i], outputData[i]); }

        cairo_surface_destroy(surface);
        cairo_surface_destroy(outputSurface);
    }

    void testReadString() {
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
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            std::string output = stream.readString();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    void testReadSizeT() {
        std::vector<size_t> sizeTToTest{0, 1, 42, 1337, 10000000, 10000000000};

        std::vector<std::pair<std::string, size_t>> testData;
        testData.reserve(sizeTToTest.size());
        for (size_t number: sizeTToTest) { testData.emplace_back(serializeSizeT(number), number); }

        for (auto&& data: testData) {
            std::string& str = data.first;
            size_t x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            size_t output = stream.readSizeT();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    void testReadInt() {
        std::vector<int> intToTest{0, 1, -1, 42, -50000, -1337, 10000};

        std::vector<std::pair<std::string, int>> testData;
        testData.reserve(intToTest.size());
        for (auto&& number: intToTest) { testData.emplace_back(serializeInt(number), number); }

        for (auto&& data: testData) {
            std::string& str = data.first;
            int x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            int output = stream.readInt();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }


    void testReadDouble() {
        std::vector<double> doubleToTest{0., 0.5, 42., 46.5, -85.2, -1337, 1e50};

        std::vector<std::pair<std::string, double>> testData;
        testData.reserve(doubleToTest.size());
        for (auto&& number: doubleToTest) { testData.emplace_back(serializeDouble(number), number); }

        for (auto&& data: testData) {
            std::string& str = data.first;
            double dbl = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], (int)str.size() + 1));
            double output = stream.readDouble();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(dbl, output, 0.001);
        }
    }

    void assertStrokeEquality(const Stroke& stroke1, const Stroke& stroke2) {
        CPPUNIT_ASSERT_EQUAL(stroke1.getAudioFilename(), stroke2.getAudioFilename());
        CPPUNIT_ASSERT_EQUAL(stroke1.getToolType(), stroke2.getToolType());
        CPPUNIT_ASSERT_EQUAL(stroke1.getFill(), stroke2.getFill());
        CPPUNIT_ASSERT_EQUAL(stroke1.getWidth(), stroke2.getWidth());

        double avgPressure1 = stroke1.getAvgPressure();
        double avgPressure2 = stroke2.getAvgPressure();

        if (!std::isnan(avgPressure1)) {
            CPPUNIT_ASSERT_DOUBLES_EQUAL(avgPressure1, avgPressure2, 1e-8);
        } else {
            CPPUNIT_ASSERT(std::isnan(avgPressure2));
        }

        std::vector<Point> points1 = stroke1.getPointVector();
        std::vector<Point> points2 = stroke2.getPointVector();

        CPPUNIT_ASSERT_EQUAL(points1.size(), points2.size());
        for (size_t i = 0; i < points1.size(); ++i) { CPPUNIT_ASSERT(points1[i].equalsPos(points2[i])); }
    }

    void testReadStroke() {
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectIOStreamTest);
