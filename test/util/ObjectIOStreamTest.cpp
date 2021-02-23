#include <vector>
#include <tuple>
#include <string>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <util/serializing/ObjectInputStream.h>
#include <util/serializing/ObjectOutputStream.h>
#include <util/serializing/BinObjectEncoding.h>
#include <util/serializing/HexObjectEncoding.h>

using namespace std;

class ObjectIOStreamTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ObjectIOStreamTest);
    CPPUNIT_TEST(testReadDouble);
    CPPUNIT_TEST(testReadInt);
    CPPUNIT_TEST(testReadSizeT);
    CPPUNIT_TEST_SUITE_END();


    public:
    void setUp() {
    }
    void tearDown() {}



    string prepareSizeT(size_t x) {
            ObjectOutputStream outStream(new BinObjectEncoding);
            outStream.writeSizeT(x);
            auto outStr = outStream.getStr();
            return string(outStr->str, outStr->len);
    }

    string prepareDouble(double x) {
            ObjectOutputStream outStream(new BinObjectEncoding);
            outStream.writeDouble(x);
            auto outStr = outStream.getStr();
            return string(outStr->str, outStr->len);
    }

    string prepareInt(int x) {
            ObjectOutputStream outStream(new BinObjectEncoding);
            outStream.writeInt(x);
            auto outStr = outStream.getStr();
            return string(outStr->str, outStr->len);
    }


    void testReadSizeT() {
        vector<size_t> intToTest = {0, 1, 42, 1337, 10000000, 10000000000};

        vector<pair<string, size_t>> testData;
        for (size_t number: intToTest) {
            testData.push_back({prepareSizeT(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            size_t x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], str.size()+1));
            size_t output = stream.readSizeT();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    void testReadInt() {
        vector<int> intToTest = {0, 1, -1, 42, -50000, -1337, 10000};

        vector<pair<string, int>> testData;
        for (int number: intToTest) {
            testData.push_back({prepareInt(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            int x = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], str.size()+1));
            int output = stream.readInt();
            CPPUNIT_ASSERT_EQUAL(x, output);
        }
    }

    
    void testReadDouble() {
        vector<double> doubleToTest = {0., 0.5, 42., 46.5, -85.2, -1337, 1e50};

        vector<pair<string, double>> testData;
        for (double number: doubleToTest) {
            testData.push_back({prepareDouble(number), number});
        }

        for (auto data: testData) {
            string str = data.first;
            double dbl = data.second;

            ObjectInputStream stream;
            // The +1 stands for the \0 character
            CPPUNIT_ASSERT(stream.read(&str[0], str.size()+1));
            double output = stream.readDouble();
            CPPUNIT_ASSERT_DOUBLES_EQUAL(dbl, output, 0.001);
        }
    }
    
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(ObjectIOStreamTest);
