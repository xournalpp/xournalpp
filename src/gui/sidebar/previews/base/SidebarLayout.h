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

#include <XournalType.h>

#include <gtk/gtk.h>

class SidebarPreviewBase;

class SidebarLayout
{
public:
	SidebarLayout();
	virtual ~SidebarLayout();

public:
	/**
	 * Layouts the sidebar
	 */
	void layout(SidebarPreviewBase* sidebar);

private:
	};
