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

#include "BaseElementView.h"
#include "BackgroundSelectDialogBase.h"

#include <XournalType.h>

class PdfPagesDialog : public BackgroundSelectDialogBase
{
public:
	PdfPagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
	virtual ~PdfPagesDialog();

public:
	virtual void show(GtkWindow* parent);
	void updateOkButton();
	double getZoom();
	int getSelectedPage();

private:
	static void onlyNotUsedCallback(GtkToggleButton* tb, PdfPagesDialog* dlg);
	static void okButtonCallback(GtkButton* button, PdfPagesDialog* dlg);

private:
	};
