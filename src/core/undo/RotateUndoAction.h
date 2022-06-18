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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Control;
class Element;

class RotateUndoAction: public UndoAction {
public:
    RotateUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0, double rotation);
    ~RotateUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

private:
    void applyRotation(double rotation);

private:
    std::vector<Element*> elements;

    double x0;
    double y0;
    double rotation = 0;
};
