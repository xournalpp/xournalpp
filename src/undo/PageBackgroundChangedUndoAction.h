/*
 * Xournal++
 *
 * Undo action for background change
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/BackgroundImage.h"
#include "model/PageRef.h"

#include "UndoAction.h"


class PageBackgroundChangedUndoAction: public UndoAction {
public:
    PageBackgroundChangedUndoAction(const PageRef& page, const PageType& origType, int origPdfPage,
                                    BackgroundImage origBackgroundImage, double origW, double origH);
    virtual ~PageBackgroundChangedUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);

    virtual string getText();

private:
    PageType origType;
    int origPdfPage;
    BackgroundImage origBackgroundImage;
    double origW;
    double origH;

    PageType newType;
    int newPdfPage = -1;
    BackgroundImage newBackgroundImage;
    double newW = 0;
    double newH = 0;
};
