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

enum ExportFormtType {
	EXPORT_FORMAT_PDF, EXPORT_FORMAT_EPS, EXPORT_FORMAT_SVG, EXPORT_FORMAT_PNG

};

class ExportDialog: public GladeGui {
public:
	ExportDialog(int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	void show();
	GList * getRange();
	int getResolution();
	ExportFormtType getFormatType();

private:
	void handleData();

private:
	int pageCount;
	int currentPage;
	int resolution;
	ExportFormtType type;

	GList * range;
};

#endif /* __EXPORTDIALOG_H__ */
