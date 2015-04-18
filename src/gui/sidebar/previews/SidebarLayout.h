/*
 * Xournal++
 *
 * Sidebar preview layout
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include <gtk/gtk.h>
#include <XournalType.h>

class SidebarPreviews;

class SidebarLayout
{
public:
	SidebarLayout();
	virtual ~SidebarLayout();

public:
	/**
	 * Layouts the sidebar
	 */
	void layout(SidebarPreviews* sidebar);

private:
	XOJ_TYPE_ATTRIB;

};
