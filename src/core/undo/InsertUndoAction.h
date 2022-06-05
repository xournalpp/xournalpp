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

#include "UndoAction.h"

class Element;
class Layer;

class InsertUndoAction: public UndoAction {
public:
    InsertUndoAction(const PageRef& page, Layer* layer, Element* element);
    ~InsertUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    Element* element;
};

class InsertsUndoAction: public UndoAction {
public:
    InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<Element*> elements);
    ~InsertsUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    std::vector<Element*> elements;
};
