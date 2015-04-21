/*
 * Xournal++
 *
 * Sidebar preview layout
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>
#include <XournalType.h>

#include "../../../model/PageRef.h"

class Control;

class SidebarToolbar
{
public:
	SidebarToolbar(Control* control);
	virtual ~SidebarToolbar();

public:
	GtkWidget* getWidget();

	/**
	 * Sets the button enabled / disabled
	 */
	void setButtonEnabled(bool enableUp, bool enableDown, bool enableCopy,
						bool enableDelete, PageRef currentPage);

private:
	XOJ_TYPE_ATTRIB;

private:
	static void btUpClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
	static void btDownClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
	static void btCopyClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
	static void btDeleteClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);

private:

	/**
	 * The Application Controller
	 */
	Control* control;

	/**
	 * The current selected page
	 */
	PageRef currentPage;

	/**
	 * The Toolbar
	 */
	GtkToolbar* toolbar;

	/**
	 * Button move Page up
	 */
	GtkToolItem* btUp;

	/**
	 * Button move Page down
	 */
	GtkToolItem* btDown;

	/**
	 * Button copy current page
	 */
	GtkToolItem* btCopy;

	/**
	 * Button delete page
	 */
	GtkToolItem* btDelete;
};
