/*
 * Xournal++
 *
 * Undo action for text editing
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
class Text;
class TextEditor;

class TextUndoAction: public UndoAction {
public:
    TextUndoAction(const PageRef& page, Layer* layer, Text* text, std::string lastText, TextEditor* textEditor);
    virtual ~TextUndoAction();

public:
    virtual bool undo(Control* control);
    virtual bool redo(Control* control);

    virtual std::string getText();

    std::string getUndoText();

    void textEditFinished();

private:
    Layer* layer;
    Text* text;
    std::string lastText;
    std::string newText;

    TextEditor* textEditor;
};
