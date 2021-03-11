/*
 * Xournal++
 *
 * Undo move action (EditSelection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class Redrawable;
class XojPage;

class MoveUndoAction: public UndoAction {
public:
    MoveUndoAction(Layer* sourceLayer, const PageRef& sourcePage, std::vector<Element*>* selected, double mx, double my,
                   Layer* targetLayer, PageRef targetPage);
    virtual ~MoveUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);
    std::vector<PageRef> getPages();
    virtual std::string getText();

private:
    void switchLayer(std::vector<Element*>* entries, Layer* oldLayer, Layer* newLayer);
    void repaint();
    void move();

private:
    std::vector<Element*> elements;
    PageRef targetPage;

    Layer* sourceLayer = nullptr;
    Layer* targetLayer = nullptr;

    std::string text;

    double dx = 0;
    double dy = 0;
};
