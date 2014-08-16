/*
 * Xournal++
 *
 * Sidebar preview layout
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SIDEBARTOOLBAR_H__
#define __SIDEBARTOOLBAR_H__

#include <gtk/gtk.h>
#include <XournalType.h>

#include "../../../model/PageRef.h"

#include "../../GladeGui.h"

class Control;

class SidebarToolbar
{
public:
	SidebarToolbar(Control* control, GladeGui* gui);
	virtual ~SidebarToolbar();

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

#endif /* __SIDEBARTOOLBAR_H__ */
