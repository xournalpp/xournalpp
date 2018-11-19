/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/jobs/ExportFormtType.h"
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"

#include <PageRange.h>
#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class ExportDialog : public GladeGui
{
public:
	ExportDialog(GladeSearchpath* gladeSearchPath, Settings* settings, int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	virtual void show(GtkWindow* parent);

	PageRangeVector getRange();
	int getPngDpi();
	ExportFormtType getFormatType();

	string getFilePath();

private:
	bool validate();
	void handleData();

	static gboolean rangeFocused(GtkWidget* widget, GdkEvent* event, ExportDialog* dlg);

	static void fileTypeSelected(GtkTreeView* treeview, ExportDialog* dlg);

	bool validFilename();
	bool validExtension();

	bool fileTypeByExtension();

private:
	XOJ_TYPE_ATTRIB;


	int pageCount;
	int currentPage;
	int resolution;
	ExportFormtType type;

	PageRangeVector range;

	Settings* settings;

	enum ColIndex
	{
		COL_FILEDESC = 0,
		COL_EXTENSION = 1,
		COL_TYPE = 2
	};
};
