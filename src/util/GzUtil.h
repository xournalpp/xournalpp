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

#include "Path.h"
#include <zlib.h>

class GzUtil
{
private:
	GzUtil();
	virtual ~GzUtil();

public:
	static gzFile openPath(const Path& path, const string& flags);
};

