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
        fs::path a = PathUtil::fromUri("http://localhost/test.txt");
        CPPUNIT_ASSERT_EQUAL(true, a.empty());
        fs::path b = PathUtil::fromUri("file://invalid");
        CPPUNIT_ASSERT_EQUAL(true, b.empty());
    }

    void testPathFromUri() {
        fs::path b = PathUtil::fromUri("file:///tmp/test.txt");
        CPPUNIT_ASSERT_EQUAL(false, b.empty());
        CPPUNIT_ASSERT_EQUAL(G_DIR_SEPARATOR_S + string("tmp") + G_DIR_SEPARATOR_S + string("test.txt"), b.string());
    }

    void testClearExtensions() {
        fs::path a("C:\\test\\abc\\xyz.txt");
        PathUtil::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(string("C:\\test\\abc\\xyz.txt"), a.string());

        fs::path b("/test/asdf.TXT");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.TXT"), b.string());
        PathUtil::clearExtensions(b, ".txt");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.asdf/asdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.asdf/asdf"), b.string());

        b = fs::path("/test/asdf.PDF");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = fs::path("/test/asdf.PDF.xoj");
        PathUtil::clearExtensions(b, ".Pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.pdf.pdf");
        PathUtil::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());

        b = fs::path("/test/asdf.xopp.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.xopp"), b.string());

        b = fs::path("/test/asdf.PDF.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = fs::path("/test/asdf.SVG.xopp");
        PathUtil::clearExtensions(b, ".svg");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.xoj");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.xopp");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.pdf");
        PathUtil::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);
