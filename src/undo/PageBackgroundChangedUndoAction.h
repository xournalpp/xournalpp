/*
 * Xournal++
 *
 * Undo action for background change
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "UndoAction.h"
#include "../model/PageRef.h"
#include "../model/BackgroundImage.h"

class PageBackgroundChangedUndoAction : public UndoAction
{
public:
	PageBackgroundChangedUndoAction(PageRef page, BackgroundType origType,
									int origPdfPage,
									BackgroundImage origBackgroundImage, double origW, double origH);
	virtual ~PageBackgroundChangedUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	BackgroundType origType;
	int origPdfPage;
	BackgroundImage origBackgroundImage;
	double origW;
	double origH;

	BackgroundType newType;
	int newPdfPage;
	BackgroundImage newBackgroundImage;
	double newW;
	double newH;
};
