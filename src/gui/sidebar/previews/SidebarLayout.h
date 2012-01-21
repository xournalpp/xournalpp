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

class SidebarLayout {
public:
	SidebarLayout();
	virtual ~SidebarLayout();

private:
	XOJ_TYPE_ATTRIB;

};

#endif /* __SIDEBARLAYOUT_H__ */
