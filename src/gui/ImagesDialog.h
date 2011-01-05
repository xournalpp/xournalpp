/*
 * Xournal++
 *
 * Dialog to select a Image (to insert as background)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
#ifndef __IMAGESDIALOG_H__
#define __IMAGESDIALOG_H__

#include "GladeGui.h"
#include "../model/Document.h"
#include "../control/Settings.h"

class ImagesDialog: public GladeGui {
public:
	ImagesDialog(Document * doc, Settings * settings);
	virtual ~ImagesDialog();

	void show();
	void setBackgroundWhite();

	void setSelected(int selected);

	BackgroundImage * getSelectedImage();
	bool shouldShowFilechooser();

	Settings * getSettings();
private:
	void layout();
	void updateOkButton();

	static void sizeAllocate(GtkWidget *widget, GtkRequisition *requisition, ImagesDialog * dlg);
	static void okButtonCallback(GtkButton *button, ImagesDialog * dlg);
	static void filechooserButtonCallback(GtkButton *button, ImagesDialog * dlg);

private:
	bool backgroundInitialized;

	Settings * settings;

	int selected;
	int lastWidth;

	int selectedPage;

	GList * images;

	GtkWidget * scrollPreview;
	GtkWidget * widget;
};

#endif /* __IMAGESDIALOG_H__ */
