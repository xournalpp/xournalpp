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

#include <config-test.h>
#include <gtest/gtest.h>

#include "model/Document.h"
#include "model/DocumentHandler.h"

TEST(DocumentName, testUTF8) {
    DocumentHandler dh;
    Document doc(&dh);
    fs::path p;
    bool failed = false;
    auto trything = [&](Document::DocumentType t) {
        try {
            p = doc.createSaveFilename(t, "%% %Y %EY %B %A", "%{name} %Y %EY %B %A");
            std::cout << "Resulting path: " << p << std::endl;
            if (!g_utf8_validate(p.u8string().c_str(), -1, nullptr)) {
                failed = true;
                std::cout << "This path yields an invalid UTF8 string" << std::endl;
            }
        } catch (const std::exception& e) {
            failed = true;
            std::cout << e.what() << std::endl;
        }
    };
    trything(Document::PDF);
    trything(Document::XOPP);
    doc.setFilepath(fs::u8path(u8"ùèçüûin/ë€ds测试q.xopp"));
    trything(Document::PDF);
    trything(Document::XOPP);
    if (failed) {
        FAIL();
    }
}
