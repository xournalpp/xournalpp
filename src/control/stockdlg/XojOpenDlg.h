/*
 * Xournal++
 *
 * GTK Open dialog to select XOJ (or PDF) file with preview
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/settings/Settings.h"
#include <Path.h>

#include <gtk/gtk.h>

class XojOpenDlg
{
public:
	XojOpenDlg(GtkWindow* win, Settings* settings);
	virtual ~XojOpenDlg();

public:
	Path showOpenDialog(bool pdf, bool& attachPdf);
	Path showOpenTemplateDialog();

protected:
	void addFilterAllFiles();
	void addFilterPdf();
	void addFilterXoj();
	void addFilterXopp();
	void addFilterXopt();

	Path runDialog();

private:
	static void updatePreviewCallback(GtkFileChooser* fileChooser, void* userData);

private:
	GtkWidget* dialog;

	GtkWindow* win;
	Settings* settings;
};
