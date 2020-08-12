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
#include <filesystem>

#include "PathUtil.h"

using namespace std;

class PathTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PathTest);

    CPPUNIT_TEST(testUnsupportedUri);
    CPPUNIT_TEST(testPathFromUri);
    CPPUNIT_TEST(testParentPath);
    CPPUNIT_TEST(testFilename);
    CPPUNIT_TEST(testHasExtension);
    CPPUNIT_TEST(testClearExtensions);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

    void testUnsupportedUri() {
        std::filesystem::path a = PathUtil::fromUri("http://localhost/test.txt");
        CPPUNIT_ASSERT_EQUAL(true, a.empty());
        std::filesystem::path b = PathUtil::fromUri("file://invalid");
        CPPUNIT_ASSERT_EQUAL(true, b.empty());
    }

    void testPathFromUri() {
        std::filesystem::path b = PathUtil::fromUri("file:///tmp/test.txt");
        CPPUNIT_ASSERT_EQUAL(false, b.empty());
        CPPUNIT_ASSERT_EQUAL(G_DIR_SEPARATOR_S + string("tmp") + G_DIR_SEPARATOR_S + string("test.txt"), b.string());
    }

    void testParentPath() {
        std::filesystem::path a("C:\\test\\abc\\xyz.txt");
        CPPUNIT_ASSERT_EQUAL(string("C:\\test\\abc"), a.parent_path().string());
        CPPUNIT_ASSERT_EQUAL(string("C:\\test"), a.parent_path().parent_path().string());

        std::filesystem::path b("/temp/test/asdf.txt");
        CPPUNIT_ASSERT_EQUAL(string("/temp/test"), b.parent_path().string());
        CPPUNIT_ASSERT_EQUAL(string("/temp"), b.parent_path().parent_path().string());
    }

    void testFilename() {
        std::filesystem::path a("C:\\test\\abc\\xyz.txt");
        CPPUNIT_ASSERT_EQUAL(string("xyz.txt"), a.filename().string());
        CPPUNIT_ASSERT_EQUAL(string("abc"), a.parent_path().filename().string());

        std::filesystem::path b("/temp/test/asdf.txt");
        CPPUNIT_ASSERT_EQUAL(string("asdf.txt"), b.filename().string());
        CPPUNIT_ASSERT_EQUAL(string("test"), b.parent_path().filename().string());
    }

    void testHasExtension() {
        // std::filesystem::path a("C:\\test\\abc\\xyz.txt");
        std::filesystem::path a("/test/abc/xyz.txt");
        CPPUNIT_ASSERT_EQUAL(string(".txt"), a.extension().string());

        // std::filesystem::path b("C:\\test\\abc\\xyz.TXT");
        std::filesystem::path b("test/abc/xyz.TXT");
        CPPUNIT_ASSERT_EQUAL(string(".txt"), b.extension().string());

        CPPUNIT_ASSERT_EQUAL(false, a.extension() == ".");
        CPPUNIT_ASSERT_EQUAL(false, a.extension() == "xyz");
    }

    void testClearExtensions() {
        std::filesystem::path a("C:\\test\\abc\\xyz.txt");
        PathUtil::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(string("C:\\test\\abc\\xyz.txt"), a.string());

        std::filesystem::path b("/test/asdf.TXT");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.TXT"), b.string());
        PathUtil::clearExtensions(b, ".txt");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.asdf/asdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.asdf/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.PDF");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = std::filesystem::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b, ".Pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.pdf.pdf");
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());

        b = std::filesystem::path("/test/asdf.xopp.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.xopp"), b.string());

        b = std::filesystem::path("/test/asdf.PDF.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = std::filesystem::path("/test/asdf.SVG.xopp");
        PathUtil::clearExtensions(b, ".svg");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = std::filesystem::path("/test/asdf.pdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());
    }

    void testOperators() {
        std::filesystem::path a("/test/a");
        std::filesystem::path b = a / "foo.pdf";
        CPPUNIT_ASSERT_EQUAL(string("/test/a"), a.string());
        CPPUNIT_ASSERT_EQUAL(string("/test/foo.pdf"), b.string());

        a /= "bar.pdf";
        CPPUNIT_ASSERT_EQUAL(string("/test/a/bar.pdf"), a.string());
        // b should not be affected by a
        CPPUNIT_ASSERT_EQUAL(string("/test/foo.pdf"), b.string());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);
