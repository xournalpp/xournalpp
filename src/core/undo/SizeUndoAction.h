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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Layer;
class SizeUndoActionEntry;
class Stroke;
class Control;

class SizeUndoAction: public UndoAction {
public:
    SizeUndoAction(const PageRef& page, Layer* layer);
    ~SizeUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;
    std::string getText() override;

    void addStroke(Stroke* s, double originalWidth, double newWidth, std::vector<double> originalPressure,
                   std::vector<double> newPressure, int pressureCount);

public:
    static std::vector<double> getPressure(Stroke* s);

private:
    std::vector<SizeUndoActionEntry*> data;

    Layer* layer;
};
