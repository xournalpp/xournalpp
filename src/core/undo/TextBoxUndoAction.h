/*
 * Xournal++
 *
 * Undo action for insert (write text, draw stroke...)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Element;
class Layer;
class Control;

class TextBoxUndoAction: public UndoAction {
public:
    TextBoxUndoAction(const PageRef& page, Layer* layer, Element* element, Element* oldelement);
    ~TextBoxUndoAction() override;

public:
    auto undo(Control* control) -> bool override;
    auto redo(Control* control) -> bool override;

    auto getText() -> std::string override;

private:
    Layer* layer;
    Element* element;
    Element* oldelement;
};
