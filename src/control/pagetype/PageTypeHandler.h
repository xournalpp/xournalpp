/*
 * Xournal++
 *
 * Handles differnt page background types
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageType.h"

#include <XournalType.h>

class PageTypeInfo {
public:
	PageType page;
	string name;
};

class GladeSearchpath;

class PageTypeHandler
{
public:
	PageTypeHandler(GladeSearchpath* gladeSearchPath);
	virtual ~PageTypeHandler();

public:
	vector<PageTypeInfo*>& getPageTypes();

private:
	void addPageTypeInfo(string name, string format, string config);
	bool parseIni(string filename);
	void loadFormat(GKeyFile* config, const char* group);

private:
	XOJ_TYPE_ATTRIB;

	vector<PageTypeInfo*> types;
};
