/*
 * Xournal++
 *
 * Standalone executable to benchmark or analyze file loading
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <exception>
#include <iostream>
#include <sstream>

#include "control/xojfile/LoadHandler.h"
#include "util/serdesstream.h"

#include "filesystem.h"

template <typename T>
static void do_not_optimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

auto main(int argc, char* argv[]) -> int {
    if (argc < 2 || argc > 3) {
        // 1st argument: argv[1] - filename
        // 2nd argument: argv[2] - optional repeat count (default 1)
        std::cerr << "Usage: " << argv[0] << " <file.xopp> [repeat_count]\n";
        return 1;
    }

    const fs::path filepath = argv[1];

    int repeat = 1;
    if (argc == 3) {
        // Parse repeat count
        auto is = serdes_stream<std::istringstream>(argv[2]);
        if (!(is >> repeat) || !(is >> std::ws).eof() || repeat <= 0) {
            std::cerr << "Invalid repeat count: " << argv[2] << '\n';
            return 1;
        }
    }

    try {
        for (int i = 0; i < repeat; ++i) {
            LoadHandler handler;
            const auto doc = handler.loadDocument(filepath);
            do_not_optimize(doc.get());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading file: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
