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

class BackgroundImage;

class ImagesDialog : public BackgroundSelectDialogBase
{
public:
	ImagesDialog(GladeSearchpath* gladeSearchPath, Document* doc, Settings* settings);
	virtual ~ImagesDialog();

public:
	BackgroundImage getSelectedImage();
	bool shouldShowFilechooser();
	virtual void show(GtkWindow* parent);

private:
	/**
	 * Load all images form all pages
	 */
	void loadImagesFromPages();

	/**
	 * Check if this image is already displayed
	 */
	bool isImageAlreadyInTheList(BackgroundImage& image);

	static void okButtonCallback(GtkButton* button, ImagesDialog* dlg);
	static void filechooserButtonCallback(GtkButton* button, ImagesDialog* dlg);

private:
	};
