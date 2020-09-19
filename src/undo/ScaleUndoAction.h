/*
 * Xournal++
 *
 * Undo action for rescale (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class ScaleUndoAction: public UndoAction {
public:
    ScaleUndoAction(const PageRef& page, vector<Element*>* elements, double x0, double y0, double fx, double fy,
                    double rotation, bool restoreLineWidth);
    virtual ~ScaleUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);
    virtual string getText();

private:
    void applyScale(double fx, double fy, bool restoreLineWidth);

private:
    vector<Element*> elements;

    double x0;
    double y0;
    double fx;
    double fy;
    double rotation;
    bool restoreLineWidth;
};
