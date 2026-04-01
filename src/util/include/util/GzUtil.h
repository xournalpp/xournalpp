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

#include <string>  // for string

#include <zlib.h>  // for gzFile

#include "filesystem.h"  // for path

class GzUtil {
private:
    GzUtil();
    virtual ~GzUtil();

public:
    static gzFile openPath(const fs::path& path, const std::string& flags);
};
