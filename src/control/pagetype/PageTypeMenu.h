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

class PageTypeMenuChangeListener
{
public:
	virtual void pageSelected(PageTypeInfo* info) = 0;
	virtual ~PageTypeMenuChangeListener();
};

class PageTypeMenu
{
public:
	PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showSpecial = true);
	virtual ~PageTypeMenu();

public:
	GtkWidget* getMenu();
	PageType getSelected();
	void loadDefaultPage();
	void setSelected(PageType selected);
	void setListener(PageTypeMenuChangeListener* listener);

private:
	void initDefaultMenu();
	void addMenuEntry(PageTypeInfo* t);
	void entrySelected(PageTypeInfo* t);

private:
	XOJ_TYPE_ATTRIB;

	bool showSpecial;

	GtkWidget* menu;
	PageTypeHandler* types;
	Settings* settings;

	vector<MenuCallbackInfo> menuInfos;

	PageType selected;

	bool ignoreEvents;

	PageTypeMenuChangeListener* listener;
};
