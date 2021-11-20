/*
 * Xournal++
 *
 * Undo action resize
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
class SizeUndoActionEntry;
class Stroke;

class SizeUndoAction: public UndoAction {
public:
    SizeUndoAction(const PageRef& page, Layer* layer);
    virtual ~SizeUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);
    virtual std::string getText();

    void addStroke(Stroke* s, double originalWidth, double newWidth, std::vector<double> originalPressure,
                   std::vector<double> newPressure, int pressureCount);

public:
    static std::vector<double> getPressure(Stroke* s);

private:
    std::vector<SizeUndoActionEntry*> data;

    Layer* layer;
};
