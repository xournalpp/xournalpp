/*
 * Xournal++
 *
 * Handles page selection menu
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageType.h"

#include <XournalType.h>

#include <gtk/gtk.h>

#include <vector>
using std::vector;

class PageTypeHandler;
class PageTypeHandler;
class PageTypeInfo;
class Settings;

typedef struct {
	GtkWidget* entry;
	PageTypeInfo* info;
} MenuCallbackInfo;

class PageTypeMenu
{
public:
	PageTypeMenu(PageTypeHandler* types, Settings* settings);
	virtual ~PageTypeMenu();

public:
	GtkWidget* getMenu();
	PageType getSelected();
	void loadDefaultPage();

private:
	void initDefaultMenu();
	void addMenuEntry(PageTypeInfo* t);
	void entrySelected(PageTypeInfo* t);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* menu;
	PageTypeHandler* types;
	Settings* settings;

	vector<MenuCallbackInfo> menuInfos;

	PageType selected;

	bool ignoreEvents;
};
