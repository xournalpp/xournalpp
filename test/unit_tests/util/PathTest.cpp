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


#include <cctype>
#include <filesystem>

#include <gtest/gtest.h>

#include "util/PathUtil.h"
#include "util/StringUtils.h"
#include "util/raii/CStringWrapper.h"

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
    EXPECT_EQ(std::u8string_view(u8"C:/dir/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "D:\\base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"C:/dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("C:/dir/file.txt", "D:", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"C:/dir/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:/dir/file.txt", "D:/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"C:/dir/file.txt"), p.u8string());

    // Test \\ to / conversion + relative
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "C:\\dir", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\file.txt", "C:\\base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"../dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("C:\\dir\\..\\dir2\\.\\dir3\\..\\file.txt", "C:\\base",
                                 Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"../dir2/file.txt"), p.u8string());
    p = Util::normalizeAssetPath("C:\\dir\\..\\dir2\\.\\dir3\\..\\file.txt", "C:\\base",
                                 Util::PathStorageMode::AS_ABSOLUTE_PATH);
    EXPECT_EQ(std::u8string_view(u8"C:/dir2/file.txt"), p.u8string());

    auto current = fs::current_path();
    auto gen = fs::weakly_canonical((current / "..\\dir\\file.txt")).generic_u8string();
    // relative asset path
    p = Util::normalizeAssetPath("..\\dir\\file.txt", "H:", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(gen, p.u8string());

    p = Util::normalizeAssetPath(".\\file.txt", fs::current_path(), Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"file.txt"), p.u8string());
#else
    p = Util::normalizeAssetPath("/dir/file.txt", "/", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/file.txt", "/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"../dir/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/../dir2/./dir3/../file.txt", "/base", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"../dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("../../dir/../dir2/./dir3/../file.txt", {}, Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"../../dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("/dir/../dir2/./dir3/../file.txt", "/base", Util::PathStorageMode::AS_ABSOLUTE_PATH);
    EXPECT_EQ(std::u8string_view(u8"/dir2/file.txt"), p.u8string());

    p = Util::normalizeAssetPath("dir/file.txt", "dir/..", Util::PathStorageMode::AS_RELATIVE_PATH);
    EXPECT_EQ(std::u8string_view(u8"dir/file.txt"), p.u8string());

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
        EXPECT_EQ(std::u8string_view(u8"../xournalpp-test-symlink/file.txt"), p.u8string());
        fs::remove("xournalpp-test-symlink");
    } else {
        FAIL() << "File named \"xournalpp-test-symlink\" already exists";
    }

#endif
}

TEST(UtilPath, pathGFilenameConvertion) {
    auto test = [](const char8_t* str) {
        fs::path p = fs::path(str);
        auto gf = xoj::util::OwnedCString::assumeOwnership(
                g_filename_from_utf8(char_cast(str), -1, nullptr, nullptr, nullptr));

        EXPECT_EQ(p.u8string(), str);
        EXPECT_STREQ(Util::toGFilename(p).c_str(), gf.get());
        EXPECT_EQ(p, Util::fromGFilename(gf.get()));

        xoj::util::GObjectSPtr<GFile> gfile(g_file_new_for_path(gf.get()), xoj::util::adopt);
        EXPECT_TRUE(g_file_equal(Util::toGFile(p).get(), gfile.get()));
        EXPECT_EQ(fs::absolute(p), fs::absolute(Util::fromGFile(gfile.get())));

        EXPECT_EQ(fs::absolute(p), fs::absolute(Util::fromUri(Util::toUri(p).value()).value()));
    };
    test(u8"foo/µbar/Ñ/ë€ds测试q.pdf");
    test(u8"/foo/µbar/Ñ/ë€ds测试q.pdf");
}

#ifdef _WIN32

TEST(UtilPath, stdFsAbsolute) {
    EXPECT_EQ(fs::absolute("C:\\Windows\\System32"), "C:\\Windows\\System32");
    EXPECT_EQ(fs::absolute("\\\\server\\some\\path"), "\\\\server\\some\\path");
}

TEST(UtilPath, stdFsIsAbsolute) {
    EXPECT_TRUE(fs::path("C:\\Windows\\System32").is_absolute());
    EXPECT_FALSE(fs::path("C:\\Windows\\System32").is_relative());

#ifdef XOURNALPP_WRAP_STD_FS_ABSOLUTE
    bool hasCorrectImpl = false;
#else
    bool hasCorrectImpl = true;
#endif

    EXPECT_EQ(fs::path("\\\\server\\some\\path").is_absolute(), hasCorrectImpl);
    EXPECT_EQ(fs::path("\\\\server\\some\\path").is_relative(), !hasCorrectImpl);

    EXPECT_EQ(fs::path("\\\\server\\some\\path\\..\\other").is_absolute(), hasCorrectImpl);
    EXPECT_EQ(fs::path("\\\\server\\some\\path\\..\\other").is_relative(), !hasCorrectImpl);

    // too many slashes
    EXPECT_FALSE(fs::path("\\\\\\server\\some\\path").is_absolute());
    EXPECT_TRUE(fs::path("\\\\\\server\\some\\path").is_relative());

    // too few slashes
    EXPECT_FALSE(fs::path("\\server\\some\\path").is_absolute());
    EXPECT_TRUE(fs::path("\\server\\some\\path").is_relative());
}

TEST(UtilPath, stdFsWeaklyCanonical) {
    // Already canonical
    EXPECT_EQ(fs::weakly_canonical("C:\\Windows\\System32"), "C:\\Windows\\System32");

    EXPECT_EQ(fs::weakly_canonical("C:\\Windows\\System32\\..\\System32"), "C:\\Windows\\System32");

#ifdef XOURNALPP_WRAP_STD_FS_ABSOLUTE
    // UNC paths should stay if they're canonical
    EXPECT_EQ(fs::weakly_canonical("\\\\?\\C:\\Windows\\System32"), "\\\\?\\C:\\Windows\\System32");

    // otherwise, they get mangled when we use the wrapped FS functions
    fs::path mangled = fs::weakly_canonical("\\\\?\\C:\\Windows\\System32\\..\\System32");
    EXPECT_EQ(StringUtils::toLowerCase(mangled.string().substr(1)), ":\\?\\c:\\windows\\system32") << mangled;
    EXPECT_TRUE(std::isalpha(mangled.string()[0])) << mangled;
#else
    EXPECT_EQ(fs::weakly_canonical("\\\\?\\C:\\Windows\\System32"), "C:\\Windows\\System32");
    EXPECT_EQ(fs::weakly_canonical("\\\\?\\C:\\Windows\\System32\\..\\System32"), "C:\\Windows\\System32");
#endif
}

#endif  // _WIN32
