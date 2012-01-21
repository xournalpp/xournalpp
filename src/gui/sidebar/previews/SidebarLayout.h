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

#ifndef __SIDEBARLAYOUT_H__
#define __SIDEBARLAYOUT_H__

#include <gtk/gtk.h>
#include <XournalType.h>

class SidebarPreviews;

class SidebarLayout {
public:
	SidebarLayout();
	virtual ~SidebarLayout();

public:
	/**
	 * Layouts the sidebar
	 */
	void layout(SidebarPreviews * sidebar);

private:
	XOJ_TYPE_ATTRIB;

};

#endif /* __SIDEBARLAYOUT_H__ */
