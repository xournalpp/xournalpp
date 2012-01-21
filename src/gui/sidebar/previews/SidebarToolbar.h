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

class SidebarToolbar {
public:
	SidebarToolbar();
	virtual ~SidebarToolbar();

public:
	GtkWidget * getWidget();

private:
	XOJ_TYPE_ATTRIB;

private:
	GtkToolbar * toolbar;
};

#endif /* __SIDEBARTOOLBAR_H__ */
