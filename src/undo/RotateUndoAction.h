/*
 * Xournal++
 *
 * Undo action for rotate (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class RotateUndoAction: public UndoAction {
public:
    RotateUndoAction(const PageRef& page, vector<Element*>* elements, double x0, double y0, double rotation);
    virtual ~RotateUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);
    virtual string getText();

private:
    void applyRotation(double rotation);

private:
    vector<Element*> elements;

    double x0;
    double y0;
    double rotation = 0;
};
