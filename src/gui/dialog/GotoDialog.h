/*
 * Xournal++
 *
 * Goto-Page dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __GOTODIALOG_H__
#define __GOTODIALOG_H__

#include "../GladeGui.h"
#include <XournalType.h>

class GotoDialog : public GladeGui {
public:
	GotoDialog(GladeSearchpath * gladeSearchPath, int maxPage);
	virtual ~GotoDialog();

public:
	virtual void show(GtkWindow * parent);

	// returns the selected page or -1 if closed
	int getSelectedPage();

private:
	XOJ_TYPE_ATTRIB;

	int selectedPage;

};

#endif /* __GOTODIALOG_H__ */
