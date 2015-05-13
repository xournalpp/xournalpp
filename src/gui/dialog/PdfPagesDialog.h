/*
 * Xournal++
 *
 * Dialog to select a PDF page (to insert as background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/settings/Settings.h"
#include "gui/GladeGui.h"
#include "model/Document.h"

#include <XournalType.h>

#include <vector>

class PdfPage;

class PdfPagesDialog : public GladeGui
{
public:
	PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
	virtual ~PdfPagesDialog();

public:
	virtual void show(GtkWindow* parent);

	void setBackgroundWhite();
	double getZoom();

	void setSelected(int selected);

	void setPageUsed(int page);

	int getSelectedPage();

	Settings* getSettings();

private:
	void layout();
	void updateOkButton();

	static void sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, PdfPagesDialog* dlg);
	static void onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg);
	static void okButtonCallback(GtkButton* button, PdfPagesDialog* dlg);

private:
	XOJ_TYPE_ATTRIB;

	bool backgroundInitialized;

	int selected;
	int lastWidth;

	Settings* settings;

	int selectedPage;

	std::vector<PdfPage*> pages;
	bool* usedPages;
	int count;

	GtkWidget* scrollPreview;
	GtkWidget* widget;
};
