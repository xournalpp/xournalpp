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


#include <gtest/gtest.h>

#include "util/PathUtil.h"

#include "filesystem.h"

using namespace std;


TEST(UtilPath, testUnsupportedUri) {
    auto a = Util::fromUri("http://localhost/test.txt");
    EXPECT_EQ(true, !a);
    auto b = Util::fromUri("file://invalid");
    EXPECT_EQ(true, !b);
}

TEST(UtilPath, testPathFromUri) {
    auto b = Util::fromUri("file:///tmp/test.txt");
    EXPECT_EQ(false, !b);
    EXPECT_EQ(G_DIR_SEPARATOR_S + string("tmp") + G_DIR_SEPARATOR_S + string("test.txt"), b->string());
}

TEST(UtilPath, testPathIsChildOf) {
    EXPECT_TRUE(Util::isChildOrEquivalent("C:/Users/Subdir", "C:/Users"));
    EXPECT_TRUE(Util::isChildOrEquivalent("C:/Users/Subdir", "C:/Users/"));
    EXPECT_TRUE(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users/"));
    EXPECT_TRUE(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users"));
    EXPECT_TRUE(Util::isChildOrEquivalent("C:/Users/Subdir/", "C:/Users/Subdir/"));
    EXPECT_TRUE(Util::isChildOrEquivalent("D:/Users/Subdir", "D:/Users"));
    EXPECT_TRUE(!Util::isChildOrEquivalent("D:/Users/Subdir", "D:/users"));

    EXPECT_TRUE(!Util::isChildOrEquivalent("C:/A/B", "C:/B/A"));
    EXPECT_TRUE(!Util::isChildOrEquivalent("C:/B/A", "C:/A/B"));

    EXPECT_TRUE(!Util::isChildOrEquivalent("D:/Users/Subdir", "C:/Users"));
    EXPECT_TRUE(!Util::isChildOrEquivalent("D:/Users/Subdir", "C:/Users"));

    // Todo add a symlink test
}

TEST(UtilPath, testClearExtensions) {
    // These tests use the preferred separator (i.e. "\\" on Windows and "/" on POSIX)
    auto a = fs::path("C:") / "test" / "abc" / "xyz.txt";
    fs::path old_path(a);
    Util::clearExtensions(a);
    EXPECT_EQ(old_path.string(), a.string());

    a = fs::path("C:") / "test" / "abc" / "xyz";
    old_path = a;
    a += ".xopp";
    Util::clearExtensions(a);
    EXPECT_EQ(old_path.string(), a.string());


    // The following tests use the generic separator which works on all systems
    auto b = fs::path("/test/asdf.TXT");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.TXT"), b.string());
    Util::clearExtensions(b, ".txt");
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.asdf/asdf");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.asdf/asdf"), b.string());

    b = fs::path("/test/asdf.PDF");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.PDF"), b.string());
    Util::clearExtensions(b, ".pdf");
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.PDF.xoj");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.PDF"), b.string());

    b = fs::path("/test/asdf.PDF.xoj");
    Util::clearExtensions(b, ".Pdf");
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.pdf.pdf");
    Util::clearExtensions(b, ".pdf");
    EXPECT_EQ(string("/test/asdf.pdf"), b.string());

    b = fs::path("/test/asdf.xopp.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.xopp"), b.string());

    b = fs::path("/test/asdf.PDF.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.PDF"), b.string());

    b = fs::path("/test/asdf.SVG.xopp");
    Util::clearExtensions(b, ".svg");
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.xoj");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.pdf");
    Util::clearExtensions(b);
    EXPECT_EQ(string("/test/asdf.pdf"), b.string());
}

TEST(UtilPath, normalizeAssetPath) {
    auto p = fs::path();

#if _WIN32
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "D:");
    EXPECT_EQ(string("C:\\dir\\file.txt"), p.string());

    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "D:\\base");
    EXPECT_EQ(string("C:\\dir\\file.txt"), p.string());

    // do not return empty if asset_path is relative
    p = Util::normalizeAssetPath("../dir/file.txt", "D:");
    EXPECT_TRUE(!p.empty());

    p = Util::normalizeAssetPath("../dir/file.txt", "D:\\base");
    EXPECT_TRUE(!p.empty());
#else
    p = Util::normalizeAssetPath("/dir/file.txt", "/");
    EXPECT_EQ(string("dir/file.txt"), p.string());

    p = Util::normalizeAssetPath("/dir/file.txt", "/base");
    EXPECT_EQ(string("../dir/file.txt"), p.string());

    // do not return empty if asset_path is relative
    p = Util::normalizeAssetPath("../dir/file.txt", "/");
    EXPECT_TRUE(!p.empty());

    p = Util::normalizeAssetPath("../dir/file.txt", "/base");
    EXPECT_TRUE(!p.empty());
#endif
}
