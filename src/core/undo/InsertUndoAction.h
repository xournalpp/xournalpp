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
class Redrawable;

class InsertUndoAction: public UndoAction {
public:
    InsertUndoAction(const PageRef& page, Layer* layer, Element* element);
    virtual ~InsertUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);

    virtual std::string getText();

private:
    Layer* layer;
    Element* element;
};

class InsertsUndoAction: public UndoAction {
public:
    InsertsUndoAction(const PageRef& page, Layer* layer, std::vector<Element*> elements);
    virtual ~InsertsUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);

    virtual std::string getText();

private:
    Layer* layer;
    std::vector<Element*> elements;
};
