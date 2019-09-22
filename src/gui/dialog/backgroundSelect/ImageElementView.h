/*
 * Xournal++
 *
 * Image view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseElementView.h"

#include "model/BackgroundImage.h"

#include <XournalType.h>

class ImageElementView : public BaseElementView
{
public:
	ImageElementView(int id, BackgroundSelectDialogBase* dlg);
	~ImageElementView();

protected:

	/**
	 * Paint the contents (without border / selection)
	 */
	virtual void paintContents(cairo_t* cr);

	/**
	 * Get the width in pixel, without shadow / border
	 */
	virtual int getContentWidth();

	/**
	 * Get the height in pixel, without shadow / border
	 */
	virtual int getContentHeight();

	/**
	 * Will be called before getContentWidth() / getContentHeight(), can be overwritten
	 */
	virtual void calcSize();

private:
	double zoom = 1;

	BackgroundImage backgroundImage;
	int width = -1;
	int height = -1;

	friend class ImagesDialog;
};
