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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef
#include "util/Color.h"     // for Color

#include "UndoAction.h"  // for UndoAction

class ColorUndoActionEntry;
class Element;
class Layer;
class Control;

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
