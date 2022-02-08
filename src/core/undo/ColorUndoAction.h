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


class ColorUndoActionEntry;
class Element;
class Layer;
class Redrawable;

class ColorUndoAction: public UndoAction {
public:
    ColorUndoAction(const PageRef& page, Layer* layer);
    ~ColorUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

    void addStroke(Element* e, Color originalColor, Color newColor);

private:
    std::vector<ColorUndoActionEntry*> data;
    Layer* layer;
};
