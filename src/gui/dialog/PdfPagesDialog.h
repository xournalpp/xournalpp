/*
 * Xournal++
 *
 * Dialog to select a PDF page (to insert as background)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __PDFPAGESDIALOG_H__
#define __PDFPAGESDIALOG_H__

#include "../GladeGui.h"
#include "../../model/Document.h"
#include "../../control/settings/Settings.h"
#include "../../util/XournalType.h"

class PdfPagesDialog: public GladeGui {
public:
	PdfPagesDialog(GladeSearchpath * gladeSearchPath, Document * doc, Settings * settings);
	virtual ~PdfPagesDialog();

public:
	void show();
	void setBackgroundWhite();
	double getZoom();

	void setSelected(int selected);

	void setPageUsed(int page);

	int getSelectedPage();

	Settings * getSettings();

private:
	void layout();
	void updateOkButton();

	static void sizeAllocate(GtkWidget * widget, GtkRequisition * requisition, PdfPagesDialog * dlg);
	static void onlyNotUsedCallback(GtkToggleButton * tb, PdfPagesDialog * dlg);
	static void okButtonCallback(GtkButton * button, PdfPagesDialog * dlg);

private:
	XOJ_TYPE_ATTRIB;

	bool backgroundInitialized;

	int selected;
	int lastWidth;

	Settings * settings;

	int selectedPage;

	GList * pages;
	bool * usedPages;
	int count;

	GtkWidget * scrollPreview;
	GtkWidget * widget;
};

#endif /* __PDFPAGESDIALOG_H__ */
