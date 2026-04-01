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

#include <memory>  // for unique_ptr
#include <string>  // for string
#include <vector>  // for vector

#include "model/PageRef.h"  // for PageRef

#include "UndoAction.h"  // for UndoAction

class Layer;
class Stroke;
class Control;
class Element;

class RecognizerUndoAction: public UndoAction {
public:
    RecognizerUndoAction(const PageRef& page, Layer* layer, std::unique_ptr<Element> original, Element* recognized);
    ~RecognizerUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

private:
    Layer* layer;
    Element* original;
    std::unique_ptr<Element> originalOwned;
    Element* recognized;
    std::unique_ptr<Element> recognizedOwned;
};
