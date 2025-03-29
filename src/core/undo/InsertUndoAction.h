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
#include <vector>  // for vector

#include "model/Element.h"
#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Element;
class Layer;
class Control;

using ElementPtr = std::unique_ptr<Element>;

class InsertUndoAction: public UndoAction {
public:
    InsertUndoAction(const PageRef& page, Layer* layer, const Element* element);
    ~InsertUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    const Element* element;
    ElementPtr elementOwn;
};

class InsertsUndoAction: public UndoAction {
public:
    InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<const Element*> elements);
    ~InsertsUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    std::vector<const Element*> elements;
    std::vector<ElementPtr> elementsOwn;
};
