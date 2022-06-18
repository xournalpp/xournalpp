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

#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Layer;
class Stroke;
class Control;

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
