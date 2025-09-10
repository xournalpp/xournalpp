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


#include <filesystem>

#include <gtest/gtest.h>

#include "util/PathUtil.h"

#include "filesystem.h"


TEST(UtilPath, testUnsupportedUri) {
    auto a = Util::fromUri("http://localhost/test.txt");
    EXPECT_EQ(true, !a);
    auto b = Util::fromUri("file://invalid");
    EXPECT_EQ(true, !b);
}

TEST(UtilPath, testPathFromUri) {
    auto b = Util::fromUri("file:///tmp/test.txt");
    EXPECT_EQ(false, !b);
    EXPECT_EQ(G_DIR_SEPARATOR_S + std::string("tmp") + G_DIR_SEPARATOR_S + std::string("test.txt"), b->string());
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
    EXPECT_EQ(std::string("/test/asdf.TXT"), b.string());
    Util::clearExtensions(b, ".txt");
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.asdf/asdf");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.asdf/asdf"), b.string());

    b = fs::path("/test/asdf.PDF");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.PDF"), b.string());
    Util::clearExtensions(b, ".pdf");
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.PDF.xoj");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.PDF"), b.string());

    b = fs::path("/test/asdf.PDF.xoj");
    Util::clearExtensions(b, ".Pdf");
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.pdf.pdf");
    Util::clearExtensions(b, ".pdf");
    EXPECT_EQ(std::string("/test/asdf.pdf"), b.string());

    b = fs::path("/test/asdf.xopp.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.xopp"), b.string());

    b = fs::path("/test/asdf.PDF.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.PDF"), b.string());

    b = fs::path("/test/asdf.SVG.xopp");
    Util::clearExtensions(b, ".svg");
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.xoj");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.xopp");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf"), b.string());

    b = fs::path("/test/asdf.pdf");
    Util::clearExtensions(b);
    EXPECT_EQ(std::string("/test/asdf.pdf"), b.string());
}

TEST(UtilPath, normalizeAssetPath) {
    auto p = fs::path();

#ifdef _WIN32
    // Test \\ to / conversion + not relative

    // Test, if the implementation of std::filesystem::proximate and std::filesystem::relative resolves "/" and "\\" to
    // the same path
    EXPECT_EQ(std::filesystem::proximate("D:/home/freddy", "C:/home"),
              std::filesystem::proximate("D:\\home\\freddy", "C:\\home"));
    EXPECT_EQ(std::filesystem::relative("D:/home/freddy", "C:/home"),
              std::filesystem::relative("D:\\home\\freddy", "C:\\home"));

    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "D:", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("C:/dir/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "D:\\base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("C:/dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("C:/dir/file.txt", "D:", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("C:/dir/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:/dir/file.txt", "D:/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("C:/dir/file.txt"), p.u8string());

    // Test \\ to / conversion + relative
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "C:\\dir", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "C:\\base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("../dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("C:\\dir\\..\\dir2\\.\\dir3\\..\\file.txt", "C:\\base",
                                 Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("../dir2/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\..\\dir2\\.\\dir3\\..\\file.txt", "C:\\base",
                                 Util::PathStorageMode::AS_ABSOLUTE_PATH);
    EXPECT_EQ(std::string("C:/dir2/file.txt"), p.u8string());

    auto current = fs::current_path();
    auto gen = fs::weakly_canonical((current / "..\\dir\\file.txt")).generic_u8string();
    // relative asset path
    p = Util::normalizeAssetPath("..\\dir\\file.txt", "H:", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(gen, p.u8string());

    p = Util::normalizeAssetPath(".\\file.txt", fs::current_path(), Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("file.txt"), p.u8string());
#else
    p = Util::normalizeAssetPath("/dir/file.txt", "/", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/file.txt", "/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("../dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/../dir2/./dir3/../file.txt", "/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("../dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("../../dir/../dir2/./dir3/../file.txt", {}, Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("../../dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/../dir2/./dir3/../file.txt", "/base", Util::PathStorageMode::AS_ABSOLUTE_PATH);
    EXPECT_EQ(std::string("/dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("dir/file.txt", "dir/..", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::string("dir/file.txt"), p.u8string());

    // do not return empty if asset_path is relative
    p = Util::normalizeAssetPath("../dir/file.txt", "/", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_TRUE(!p.empty());

    p = Util::normalizeAssetPath("../dir/file.txt", "/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_TRUE(!p.empty());

    // symlinks are not resolved
    if (!fs::exists("xournalpp-test-symlink")) {
        fs::create_directory_symlink("../dir", "xournalpp-test-symlink");
        p = Util::normalizeAssetPath("xournalpp-test-symlink/file.txt", "base",
                                     Util::PathStorageMode::AS_RELATIVE_PATH);
        EXPECT_EQ(std::string("../xournalpp-test-symlink/file.txt"), p.u8string());
        fs::remove("xournalpp-test-symlink");
    } else {
        FAIL() << "File named \"xournalpp-test-symlink\" already exists";
    }

#endif
}
