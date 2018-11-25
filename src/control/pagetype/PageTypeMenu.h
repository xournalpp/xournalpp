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
class MainBackgroundPainter;

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
	PageTypeMenu(PageTypeHandler* types, Settings* settings, bool showPreview, bool showSpecial);
	virtual ~PageTypeMenu();

public:
	GtkWidget* getMenu();
	PageType getSelected();
	void loadDefaultPage();
	void setSelected(PageType selected);
	void setListener(PageTypeMenuChangeListener* listener);
	void hideCopyPage();

private:
	void initDefaultMenu();
	void addMenuEntry(PageTypeInfo* t);
	void entrySelected(PageTypeInfo* t);
	cairo_surface_t* createPreviewImage(PageType pt);

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

	int menuX;
	int menuY;

	MainBackgroundPainter* backgroundPainter;

	bool showPreview;
};
