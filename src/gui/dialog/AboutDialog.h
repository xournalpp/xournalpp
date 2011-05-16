/*
 * Xournal++
 *
 * The about dialog
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ABOUTDIALOG_H__
#define __ABOUTDIALOG_H__

#include "../GladeGui.h"
#include <XournalType.h>

class AboutDialog: public GladeGui {
public:
	AboutDialog(GladeSearchpath * gladeSearchPath);
	virtual ~AboutDialog();

	void show();

private:
	XOJ_TYPE_ATTRIB;
};

#endif /* __ABOUTDIALOG_H__ */
