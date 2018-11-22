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

#include <XournalType.h>

#include <gtk/gtk.h>

class PageTypeHandler;
class PageTypeHandler;
class PageTypeInfo;

class PageTypeMenu
{
public:
	PageTypeMenu(PageTypeHandler* types);
	virtual ~PageTypeMenu();

public:
	GtkWidget* getMenu();

private:
	void initDefaultMenu();
	void addMenuEntry(PageTypeInfo* t);

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* menu;
	PageTypeHandler* types;
};
