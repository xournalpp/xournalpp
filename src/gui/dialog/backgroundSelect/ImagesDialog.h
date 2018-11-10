/*
 * Xournal++
 *
 * Dialog to select a Image (to insert as background)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/GladeGui.h"
#include "BackgroundSelectDialogBase.h"
#include <XournalType.h>

#include <vector>

class BackgroundImage;
class ImageView;
class BackgroundImage;

typedef std::vector<ImageView*> ImageViewVector;

class ImagesDialog : public BackgroundSelectDialogBase
{
public:
	ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
	virtual ~ImagesDialog();

public:
	void setBackgroundWhite();

	void setSelected(size_t selected);

	BackgroundImage getSelectedImage();
	bool shouldShowFilechooser();

private:
	void updateOkButton();

	static void sizeAllocate(GtkWidget* widget, GtkRequisition* requisition, ImagesDialog* dlg);
	static void okButtonCallback(GtkButton* button, ImagesDialog* dlg);
	static void filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg);

private:
	XOJ_TYPE_ATTRIB;

	bool backgroundInitialized;

	size_t selected;
	int lastWidth;

	int selectedPage;

	ImageViewVector images;
};
