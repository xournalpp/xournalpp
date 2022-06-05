/*
 * Xournal++
 *
 * Undo action for stroke recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class Stroke;

class RecognizerUndoAction: public UndoAction {
public:
    RecognizerUndoAction(const PageRef& page, Layer* layer, Stroke* original, Stroke* recognized);
    ~RecognizerUndoAction() override;

public:
    void addSourceElement(Stroke* s);

    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    Stroke* recognized;
    std::vector<Stroke*> original;
};
