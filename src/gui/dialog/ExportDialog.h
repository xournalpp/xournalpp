/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "../GladeGui.h"
#include "../../control/settings/Settings.h"
#include "../../control/jobs/ExportFormtType.h"
#include <XournalType.h>
#include <PageRange.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class ExportDialog : public GladeGui
{
public:
	ExportDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
				int pageCount, int currentPage);
	virtual ~ExportDialog();

public:
	virtual void show(GtkWindow* parent);

	PageRangeVector getRange();
	int getPngDpi();
	ExportFormtType getFormatType();

	path getFilePath();

private:
	bool validate();
	void handleData();

	/**
	 * Callback for a changed selection of an output file
	 */
	static void selectionChanged(GtkFileChooser* chooser,
								gpointer user_data);

	static gboolean rangeFocused(GtkWidget* widget,
								GdkEvent* event,
								gpointer user_data);

	static void fileTypeSelected(GtkTreeView* treeview,
								gpointer user_data);

	void addFileType(const char* typeDesc,
					const char* pattern,
					gint type = 0,
					const char* filterName = NULL,
					bool select = false);

	void setupModel();

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
	GtkListStore* typesModel;
	GtkTreeView* typesView;

	enum ColIndex
	{
		COL_FILEDESC = 0,
		COL_EXTENSION = 1,
		COL_TYPE = 2
	};
};
