/*
 * Xournal++
 *
 * Gzip Helper
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <zlib.h>

#include <filesystem>

class GzUtil {
private:
    GzUtil();
    virtual ~GzUtil();

public:
    static gzFile openPath(const std::filesystem::path& path, const std::string& flags);
};
