/*
 * Xournal++
 *
 * Undo action for reflection (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class ReflectUndoAction: public UndoAction {
public:
    ReflectUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0, bool x_axis);
    virtual ~ReflectUndoAction();

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

private:
    void applyReflection(double x0, double y0, bool x_axis);

private:
    std::vector<Element*> elements;

    double x0;
    double y0;
    bool x_axis = true;
};
