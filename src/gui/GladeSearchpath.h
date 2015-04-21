/*
 * Xournal++
 *
 * Search directory for icons and Glade files
 *
 * @author andreas
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <glib.h>
#include <XournalType.h>

class GladeSearchpath
{
public:
	GladeSearchpath();
	virtual ~GladeSearchpath();

public:
	void addSearchDirectory(const char* directory);

	/**
	 * Searches for a path, the returning string has to be freed
	 */
	char* findFile(const char* subdir, const char* file);

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Search directory for icons and Glade files
	 */
	GList* directories;
};
