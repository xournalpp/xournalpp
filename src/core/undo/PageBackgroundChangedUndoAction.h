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

#include <string>  // for string

#include "model/BackgroundImage.h"  // for BackgroundImage
#include "model/PageRef.h"          // for PageRef
#include "model/PageType.h"         // for PageType
#include "util/Util.h"              // for npos

#include "UndoAction.h"  // for UndoAction

class Control;


class PageBackgroundChangedUndoAction: public UndoAction {
public:
    PageBackgroundChangedUndoAction(const PageRef& page, const PageType& origType, size_t origPdfPage,
                                    BackgroundImage origBackgroundImage, double origW, double origH);
    ~PageBackgroundChangedUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    PageType origType;
    size_t origPdfPage;
    BackgroundImage origBackgroundImage;
    double origW;
    double origH;

    PageType newType;
    size_t newPdfPage = npos;
    BackgroundImage newBackgroundImage;
    double newW = 0;
    double newH = 0;
};
