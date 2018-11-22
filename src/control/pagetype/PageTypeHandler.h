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

#include <vector>
using std::vector;

#include <string>
using std::string;

class PageTypeInfo {
public:
	PageType page;
	string name;
};

class PageTypeHandler
{
public:
	PageTypeHandler();
	virtual ~PageTypeHandler();

public:
	vector<PageTypeInfo*>& getPageTypes();

private:
	void addPageTypeInfo(string name, string format, string config);

private:
	XOJ_TYPE_ATTRIB;

	vector<PageTypeInfo*> types;
};
