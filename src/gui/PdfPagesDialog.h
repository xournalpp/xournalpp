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

#include "GladeGui.h"
#include "../model/Document.h"

class PdfPagesDialog: public GladeGui {
public:
	PdfPagesDialog(Document * doc);
	virtual ~PdfPagesDialog();

	void show();
	void setBackgroundWhite();
	double getZoom();

	void setSelected(int selected);

	void setPageUsed(int page);

	int getSelectedPage();
private:
	void layout();
	void updateOkButton();

	static void sizeAllocate(GtkWidget *widget, GtkRequisition *requisition, PdfPagesDialog * dlg);
	static void onlyNotUsedCallback(GtkToggleButton * tb, PdfPagesDialog * dlg);
	static void okButtonCallback(GtkButton *button, PdfPagesDialog * dlg);

private:
	bool backgroundInitialized;

	int selected;
	int lastWidth;

	int selectedPage;

	GList * pages;
	bool * usedPages;
	int count;

	GtkWidget * scrollPreview;
	GtkWidget * widget;
};

#endif /* __PDFPAGESDIALOG_H__ */
