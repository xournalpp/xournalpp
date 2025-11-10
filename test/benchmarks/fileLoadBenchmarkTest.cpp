/*
 * Xournal++
 *
 * Fixed input benchmark test of the file loading process
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <config-test.h>
#include <glib-2.0/glib.h>
#include <gtest/gtest.h>

#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/SaveHandler.h"
#include "model/Document.h"
#include "model/XojPage.h"

#include "filesystem.h"

void benchLoadFile(const fs::path& filename, int iterations) {
    const auto start = g_get_monotonic_time();
    for (int i = 0; i < iterations; ++i) {
        LoadHandler().loadDocument(filename);
    }
    const auto stop = g_get_monotonic_time();
    std::cout << "Loaded " << filename << ' ' << iterations << " times in " << (stop - start) / 1000 << "ms.\n";
}

TEST(FileLoadBenchmark, benchmarkHandwrittenText) {
    benchLoadFile(GET_TESTFILE(u8"benchmark/handwritten-text.xopp"), 25);
}

TEST(FileLoadBenchmark, benchmarkTypedText) { benchLoadFile(GET_TESTFILE(u8"benchmark/typed-text.xopp"), 5'000); }

TEST(FileLoadBenchmark, benchmarkLatex) { benchLoadFile(GET_TESTFILE(u8"benchmark/latex.xopp"), 50); }

fs::path createTemporaryFile(void (*buildDoc)(Document&), const fs::path& filename) {
    // Build file
    DocumentHandler dh;
    Document doc(&dh);
    buildDoc(doc);

    // Save it to a temporary path
    SaveHandler sh;
    auto tmp_path = Util::getTmpDirSubfolder() / filename;
    sh.prepareSave(&doc, tmp_path);
    sh.saveTo(tmp_path);

    return tmp_path;
}

TEST(FileLoadBenchmark, benchmarkEmpty) {
    // Create empty file (containing only one obligatory page)
    const auto tmp_path = createTemporaryFile(
            [](Document& doc) {
                PageRef page = std::make_shared<XojPage>(50, 50);
                doc.addPage(page);
            },
            u8"empty.xopp");

    // Benchmark loading time
    benchLoadFile(tmp_path, 100'000);

    // Clean up test file
    fs::remove(tmp_path);
}

TEST(FileLoadBenchmark, benchmarkManyPages) {
    // Create a 500-page file
    const auto tmp_path = createTemporaryFile(
            [](Document& doc) {
                for (int i = 0; i < 500; ++i) {
                    PageRef page = std::make_shared<XojPage>(50, 50);
                    doc.addPage(page);
                }
            },
            u8"many-pages.xopp");

    // Benchmark loading time
    benchLoadFile(tmp_path, 1000);

    // Clean up test file
    fs::remove(tmp_path);
}
