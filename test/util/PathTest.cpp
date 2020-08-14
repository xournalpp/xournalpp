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

#include <ctime>

#include <config-test.h>
#include <cppunit/extensions/HelperMacros.h>
#include <stdlib.h>
#include "filesystem.h"

#include "PathUtil.h"

using namespace std;

class PathTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PathTest);

    CPPUNIT_TEST(testUnsupportedUri);
    CPPUNIT_TEST(testPathFromUri);
    CPPUNIT_TEST(testClearExtensions);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

    void testUnsupportedUri() {
        std::cout << "Testing unsupported Uri" << std::endl;
        fs::path a = PathUtil::fromUri("http://localhost/test.txt");
        std::cout << "One" << std::endl;
        CPPUNIT_ASSERT_EQUAL(true, a.empty());
        fs::path b = PathUtil::fromUri("file://invalid");
        std::cout << "Two" << std::endl;
        CPPUNIT_ASSERT_EQUAL(true, b.empty());
    }

    void testPathFromUri() {
        std::cout << "Testing from Uri" << std::endl;
        fs::path b = PathUtil::fromUri("file:///tmp/test.txt");
        std::cout << "Test empty" << std::endl;
        CPPUNIT_ASSERT_EQUAL(false, b.empty());
        std::cout << "Test file path" << std::endl;
        CPPUNIT_ASSERT_EQUAL(G_DIR_SEPARATOR_S + string("tmp") + G_DIR_SEPARATOR_S + string("test.txt"), b.string());
    }

    void testClearExtensions() {
        std::cout << "Testing clear extensions" << std::endl;
        // These tests use the preferred separator (i.e. "\\" on Windows and "/" on POSIX)
        std::cout << "One" << std::endl;
        auto a = fs::path("C:") / "test" / "abc" / "xyz.txt";
        fs::path old_path(a);
        PathUtil::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(old_path.string(), a.string());

        std::cout << "Two" << std::endl;
        a = fs::path("C:") / "test" / "abc" / "xyz";
        old_path = a;
        a += ".xopp";
        PathUtil::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(old_path.string(), a.string());
        

        // The following tests use the generic separator which works on all systems
        std::cout << "Three" << std::endl;
        auto b = fs::path("/test/asdf.TXT");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.TXT"), b.string());
        PathUtil::clearExtensions(b, ".txt");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Four" << std::endl;
        b = fs::path("/test/asdf.asdf/asdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.asdf/asdf"), b.string());

        std::cout << "Five" << std::endl;
        b = fs::path("/test/asdf.PDF");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Six" << std::endl;
        b = fs::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        std::cout << "Seven" << std::endl;
        b = fs::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b, ".Pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Eight" << std::endl;
        b = fs::path("/test/asdf.pdf.pdf");
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());

        std::cout << "Nine" << std::endl;
        b = fs::path("/test/asdf.xopp.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.xopp"), b.string());

        std::cout << "Ten" << std::endl;
        b = fs::path("/test/asdf.PDF.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        std::cout << "Eleven" << std::endl;
        b = fs::path("/test/asdf.SVG.xopp");
        PathUtil::clearExtensions(b, ".svg");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Twelve" << std::endl;
        b = fs::path("/test/asdf.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Thirteen" << std::endl;
        b = fs::path("/test/asdf.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        std::cout << "Fourteen" << std::endl;
        b = fs::path("/test/asdf.pdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);
