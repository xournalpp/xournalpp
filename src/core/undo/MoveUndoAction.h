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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Layer;
class Control;
class Element;

class MoveUndoAction: public UndoAction {
public:
    MoveUndoAction(Layer* sourceLayer, const PageRef& sourcePage, std::vector<Element*>* selected, double mx, double my,
                   Layer* targetLayer, PageRef targetPage);
    ~MoveUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::vector<PageRef> getPages() override;
    std::string getText() override;

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
