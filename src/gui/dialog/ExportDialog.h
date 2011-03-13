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

class ExportDialog: public GladeGui {
public:
	ExportDialog(GladeSearchpath * gladeSearchPath, Settings * settings, int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	void show();
	GList * getRange();
	int getPngDpi();
	ExportFormtType getFormatType();

	String getFolder();
	String getFilename();

private:
	bool validate();
	void handleData();

private:
	int pageCount;
	int currentPage;
	int resolution;
	ExportFormtType type;

	GList * range;

	Settings * settings;
};

#endif /* __EXPORTDIALOG_H__ */
