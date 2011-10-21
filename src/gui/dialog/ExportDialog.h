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

#include "../GladeGui.h"
#include "../../control/settings/Settings.h"
#include "../../control/jobs/ExportFormtType.h"
#include <XournalType.h>

class ExportDialog: public GladeGui {
public:
	ExportDialog(GladeSearchpath * gladeSearchPath, Settings * settings, int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	virtual void show(GtkWindow * parent);

	GList * getRange();
	int getPngDpi();
	ExportFormtType getFormatType();

	String getFolder();
	String getFilename();

private:
	bool validate();
	void handleData();

private:
	XOJ_TYPE_ATTRIB;


	int pageCount;
	int currentPage;
	int resolution;
	ExportFormtType type;

	GList * range;

	Settings * settings;
};

#endif /* __EXPORTDIALOG_H__ */
