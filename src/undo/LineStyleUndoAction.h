/*
 * Xournal++
 *
 * Undo action for line style changes (Edit selection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>

#include "model/LineStyle.h"

#include "UndoAction.h"

class Stroke;
class Layer;
class Redrawable;

struct LineStyleUndoActionEntry {
    LineStyleUndoActionEntry(Stroke* s, LineStyle oldStyle, LineStyle newStyle):
            s(s), oldStyle(oldStyle), newStyle(newStyle) {}
    Stroke* s;
    LineStyle oldStyle;
    LineStyle newStyle;
};

class LineStyleUndoAction: public UndoAction {
public:
    LineStyleUndoAction(const PageRef& page, Layer* layer);
    LineStyleUndoAction(LineStyleUndoAction const&) = delete;
    LineStyleUndoAction(LineStyleUndoAction&&) = delete;
    LineStyleUndoAction& operator=(LineStyleUndoAction const&) = delete;
    LineStyleUndoAction& operator=(LineStyleUndoAction&&) = delete;
    ~LineStyleUndoAction() override = default;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    string getText() override;

    void addStroke(Stroke* s, LineStyle originalStyle, LineStyle newStyle);

private:
    std::vector<LineStyleUndoActionEntry> data;
    Layer* layer;
};
