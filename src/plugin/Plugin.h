/*
 * Xournal++
 *
 * A single Xournal++ Plugin
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class Plugin
{
public:
	Plugin(string path);
	virtual ~Plugin();

private:
	XOJ_TYPE_ATTRIB;

	/**
	 * Plugin root path
	 */
	string path;
};
