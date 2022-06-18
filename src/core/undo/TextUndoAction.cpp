#include "TextUndoAction.h"

#include <algorithm>  // for max, min
#include <memory>     // for allocator, __shared_ptr_access, __share...
#include <utility>    // for move

#include "gui/TextEditor.h"   // for TextEditor
#include "model/PageRef.h"    // for PageRef
#include "model/Text.h"       // for Text
#include "model/XojPage.h"    // for XojPage
#include "undo/UndoAction.h"  // for UndoAction
#include "util/Rectangle.h"   // for Rectangle
#include "util/i18n.h"        // for _

class Control;

using xoj::util::Rectangle;

TextUndoAction::TextUndoAction(const PageRef& page, Layer* layer, Text* text, std::string lastText,
                               TextEditor* textEditor):
        UndoAction("TextUndoAction") {
    this->page = page;
    this->layer = layer;
    this->text = text;
    this->lastText = std::move(lastText);
    this->textEditor = textEditor;
}

TextUndoAction::~TextUndoAction() = default;

auto TextUndoAction::getUndoText() -> std::string { return this->lastText; }

void TextUndoAction::textEditFinished() { this->textEditor = nullptr; }

auto TextUndoAction::getText() -> std::string { return _("Text changes"); }

auto TextUndoAction::undo(Control* control) -> bool {
    double x1 = text->getX();
    double y1 = text->getY();
    double x2 = text->getX() + text->getElementWidth();
    double y2 = text->getY() + text->getElementHeight();

    newText = text->getText();
    text->setText(lastText);
    this->textEditor->setText(lastText);

    x1 = std::min(x1, text->getX());
    y1 = std::min(y1, text->getY());
    x2 = std::max(x2, text->getX() + text->getElementWidth());
    y2 = std::max(y2, text->getY() + text->getElementHeight());

    Rectangle<double> rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    this->undone = true;
    return true;
}

auto TextUndoAction::redo(Control* control) -> bool {
    double x1 = text->getX();
    double y1 = text->getY();
    double x2 = text->getX() + text->getElementWidth();
    double y2 = text->getY() + text->getElementHeight();

    text->setText(newText);
    this->textEditor->setText(newText);

    x1 = std::min(x1, text->getX());
    y1 = std::min(y1, text->getY());
    x2 = std::max(x2, text->getX() + text->getElementWidth());
    y2 = std::max(y2, text->getY() + text->getElementHeight());

    Rectangle<double> rect(x1, y1, x2 - x1, y2 - y1);
    this->page->fireRectChanged(rect);

    this->undone = false;
    return true;
}
