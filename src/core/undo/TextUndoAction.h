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
class Text;
class TextEditor;

class TextUndoAction: public UndoAction {
public:
    TextUndoAction(const PageRef& page, Layer* layer, Text* text, std::string lastText, TextEditor* textEditor);
    ~TextUndoAction() override;

public:
    bool undo(Control* control) override;
    bool redo(Control* control) override;

    std::string getText() override;

    std::string getUndoText();

    void textEditFinished();

private:
    Layer* layer;
    Text* text;
    std::string lastText;
    std::string newText;

    TextEditor* textEditor;
};
