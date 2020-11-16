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


#include <cppunit/extensions/HelperMacros.h>

#include "PathUtil.h"
#include "filesystem.h"

using namespace std;

class PathTest: public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PathTest);

    CPPUNIT_TEST(testUnsupportedUri);
    CPPUNIT_TEST(testPathFromUri);
    CPPUNIT_TEST(testClearExtensions);
    CPPUNIT_TEST(testPathIsChildOf);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {}

    void tearDown() {}

    void testUnsupportedUri() {
        auto a = Util::fromUri("http://localhost/test.txt");
        CPPUNIT_ASSERT_EQUAL(true, !a);
        auto b = Util::fromUri("file://invalid");
        CPPUNIT_ASSERT_EQUAL(true, !b);
    }

    void testPathFromUri() {
        auto b = Util::fromUri("file:///tmp/test.txt");
        CPPUNIT_ASSERT_EQUAL(false, !b);
        CPPUNIT_ASSERT_EQUAL(G_DIR_SEPARATOR_S + string("tmp") + G_DIR_SEPARATOR_S + string("test.txt"), b->string());
    }

    void testPathIsChildOf() {
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("C:/Users/Subdir", "C:/Users"));
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("C:/Users/Subdir", "C:/Users/"));
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users/"));
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users"));
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users/Subdir/"));
        CPPUNIT_ASSERT(Util::isChildOrEquivalent("D:/Users/Subdir", "D:/Users"));
        CPPUNIT_ASSERT(!Util::isChildOrEquivalent("D:/Users/Subdir", "D:/users"));

        CPPUNIT_ASSERT(!Util::isChildOrEquivalent("C:/A/B", "C:/B/A"));
        CPPUNIT_ASSERT(!Util::isChildOrEquivalent("C:/B/A", "C:/A/B"));

        CPPUNIT_ASSERT(!Util::isChildOrEquivalent("D:/Users/Subdir", "C:/Users"));
        CPPUNIT_ASSERT(!Util::isChildOrEquivalent("D:/Users/Subdir", "C:/Users"));

        // Todo add a symlink test
    }

    void testClearExtensions() {
        // These tests use the preferred separator (i.e. "\\" on Windows and "/" on POSIX)
        auto a = fs::path("C:") / "test" / "abc" / "xyz.txt";
        fs::path old_path(a);
        Util::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(old_path.string(), a.string());

        a = fs::path("C:") / "test" / "abc" / "xyz";
        old_path = a;
        a += ".xopp";
        Util::clearExtensions(a);
        CPPUNIT_ASSERT_EQUAL(old_path.string(), a.string());


        // The following tests use the generic separator which works on all systems
        auto b = fs::path("/test/asdf.TXT");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.TXT"), b.string());
        Util::clearExtensions(b, ".txt");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.asdf/asdf");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.asdf/asdf"), b.string());

        b = fs::path("/test/asdf.PDF");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());
        Util::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.PDF.xoj");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = fs::path("/test/asdf.PDF.xoj");
        Util::clearExtensions(b, ".Pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.pdf.pdf");
        Util::clearExtensions(b, ".pdf");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());

        b = fs::path("/test/asdf.xopp.xopp");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.xopp"), b.string());

        b = fs::path("/test/asdf.PDF.xopp");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.PDF"), b.string());

        b = fs::path("/test/asdf.SVG.xopp");
        Util::clearExtensions(b, ".svg");
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.xoj");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.xopp");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf"), b.string());

        b = fs::path("/test/asdf.pdf");
        Util::clearExtensions(b);
        CPPUNIT_ASSERT_EQUAL(string("/test/asdf.pdf"), b.string());
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);
