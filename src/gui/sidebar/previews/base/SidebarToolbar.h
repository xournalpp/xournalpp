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

#include "gui/GladeGui.h"
#include "model/PageRef.h"

#include <XournalType.h>

#include <gtk/gtk.h>

class Control;

class SidebarToolbar
{
public:
	SidebarToolbar(Control* control, GladeGui* gui);
	virtual ~SidebarToolbar();

public:
	/**
	 * Sets the button enabled / disabled
	 */
	void setButtonEnabled(bool enableUp, bool enableDown, bool enableCopy, bool enableDelete, PageRef currentPage);

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
	 * Button move Page up
	 */
	GtkButton* btUp;

	/**
	 * Button move Page down
	 */
	GtkButton* btDown;

	/**
	 * Button copy current page
	 */
	GtkButton* btCopy;

	/**
	 * Button delete page
	 */
	GtkButton* btDelete;
};
