/*
 * Xournal++
 *
 * Undo action for color changes (Edit selection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include "UndoAction.h"
#include "XournalType.h"

class ColorUndoActionEntry;
class Element;
class Layer;
class Redrawable;

class ColorUndoAction: public UndoAction {
public:
    ColorUndoAction(const PageRef& page, Layer* layer);
    virtual ~ColorUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);
    virtual string getText();

    void addStroke(Element* e, Color originalColor, Color newColor);

private:
    std::vector<ColorUndoActionEntry*> data;
    Layer* layer;
};
