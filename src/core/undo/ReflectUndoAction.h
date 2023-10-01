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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction
#include <cairo.h>


class Control;
class Element;


class ReflectUndoAction: public UndoAction {
public:
    ReflectUndoAction(const PageRef& page, std::vector<Element*>* elements, double x0, double y0, cairo_matrix_t * cmatrix, bool x_axis);
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
	cairo_matrix_t * cmatrix;
    bool x_axis = true;
};
