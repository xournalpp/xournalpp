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
#include <locale>

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/Document.h"
#include "model/DocumentHandler.h"

static void test_u8_fn(Document& doc, Document::DocumentType dt, std::string_view left, std::string_view right, std::string_view expected) {
    auto p = doc.createSaveFilename(dt, left, right);
    std::cout << p << std::endl;
    ASSERT_EQ(p.u8string(), expected);
}

TEST(DocumentName, testUTF8) {
    
    try {
        std::locale("C.UTF8");
    } catch (...) {
        std::cerr << "C.UTF8 not available, skiping test..." << std::endl;
        GTEST_SKIP();
    }

    DocumentHandler dh;
    Document doc(&dh);
    test_u8_fn(doc,Document::PDF, u8"%%ç测试ôê€ß", u8"%{name}测试",u8"%ç测试ôê€ß");
    test_u8_fn(doc,Document::XOPP, u8"%%ç测试ôê€ß", u8"%{name}测试",u8"%ç测试ôê€ß");
    doc.setFilepath(fs::u8path(u8"ùèçüûin/ë€ds测试q.xopp"));
    test_u8_fn(doc,Document::PDF, u8"%%ç测试ôê€ß", u8"%{name}测试",u8"ë€ds测试q测试");
    test_u8_fn(doc,Document::XOPP, u8"%%ç测试ôê€ß", u8"%{name}测试",u8"ë€ds测试q");
}