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

#include "Path.h"

class GzUtil {
private:
    GzUtil();
    virtual ~GzUtil();

public:
    static gzFile openPath(const Path& path, const string& flags);
};
