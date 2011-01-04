/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __EXPORTDIALOG_H__
#define __EXPORTDIALOG_H__

#include "GladeGui.h"

class ExportDialog: public GladeGui {
public:
	ExportDialog();
	virtual ~ExportDialog();

	void show();
};

#endif /* __EXPORTDIALOG_H__ */
