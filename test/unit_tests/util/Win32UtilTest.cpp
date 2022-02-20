/*
 * Xournal++
 *
 * This file is part of the Xournal++ UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#ifdef _WIN32

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include <windows.h>

#include "util/Win32Util.h"

using namespace std;

namespace w32 = xoj::win32;

auto testFreeF(bool* test) -> bool {
    auto result = *test;
    *test = false;
    return result;
};

TEST(Win32Util, sysResource_common) {
    bool test1 = true;
    bool test2 = false;
    using TestResource = w32::UniqueResource<bool, testFreeF, w32::DeleterFunctionType::nonzeroIsSuccess>;

    std::cout << " - Init" << std::endl;
    TestResource r = &test1;
    EXPECT_EQ(r.get(), &test1) << "Set failed";

    std::cout << " - Change" << std::endl;
    r = &test2;
    EXPECT_EQ(r.get(), &test2) << "Change failed";
    EXPECT_FALSE(test1) << "Change - deleter not called";

    std::cout << " - Ref (no change)" << std::endl;
    auto noTouch = [](bool**) {};
    noTouch(r.oref());

    std::cout << " - Ref (change 1)" << std::endl;
    auto touch1 = [&test1](bool** test) { *test = &test1; };
#if defined(__clang__) || defined(__GNUG__)
    constexpr static const char* deathMessagePattern = "Deleter function failed";
#else
    // AFAIK MSVC runtime doesn't print any messages
    constexpr static const char* deathMessagePattern = ".*";
#endif
    EXPECT_DEATH(([&]() {
                     std::cout << "Death test" << std::endl;
                     touch1(r.oref());
                     std::cout << "...didn't die" << std::endl;
                 })(),
                 deathMessagePattern);
    EXPECT_EQ(r.get(), &test2) << "Ref change 1 - changed";

    std::cout << " - Ref (change 2)" << std::endl;
    test2 = true;
    auto touch2 = [&test1](bool** test) { *test = &test1; };
    touch2(r.oref());
    EXPECT_EQ(r.get(), &test1) << "Ref change 2 failed";
    EXPECT_FALSE(test2) << "Ref change 2 - deleter not called";

    std::cout << " - Release" << std::endl;
    r.release();
}

TEST(Win32Util, sysResource_typedef_handle) {
    w32::UniqueHandle h;
    EXPECT_FALSE(h);
    HANDLE event = CreateEventW(NULL, FALSE, FALSE, NULL);
    assert(event);
    h = event;
    EXPECT_TRUE(h);
}

TEST(Win32Util, sysResource_typedef_localMemory) {
    w32::UniqueLocal<unsigned char> h;
    EXPECT_FALSE(h);
    HLOCAL hlocal = LocalAlloc(0, 42);
    assert(hlocal);
    h = reinterpret_cast<unsigned char*>(hlocal);
    EXPECT_TRUE(h);
}

#endif  // _WIN32
