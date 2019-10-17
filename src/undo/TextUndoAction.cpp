#include "TextUndoAction.h"

#include "gui/Redrawable.h"
#include "gui/TextEditor.h"
#include "model/Layer.h"
#include "model/PageRef.h"
#include "model/Text.h"

#include <i18n.h>
#include <Rectangle.h>

TextUndoAction::TextUndoAction(PageRef page, Layer* layer, Text* text, string lastText, TextEditor* textEditor)
 : UndoAction("TextUndoAction")
{
	this->page = page;
	this->layer = layer;
	this->text = text;
	this->lastText = lastText;
	this->textEditor = textEditor;
}

TextUndoAction::~TextUndoAction()
{
}

string TextUndoAction::getUndoText()
{
	return this->lastText;
}

void TextUndoAction::textEditFinished()
{
	this->textEditor = nullptr;
}

string TextUndoAction::getText()
{
	return _("Text changes");
}

bool TextUndoAction::undo(Control* control)
{
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

	Rectangle rect(x1, y1, x2 - x1, y2 - y1);
	this->page->fireRectChanged(rect);

	this->undone = true;
	return true;
}

bool TextUndoAction::redo(Control* control)
{
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

	Rectangle rect(x1, y1, x2 - x1, y2 - y1);
	this->page->fireRectChanged(rect);

	this->undone = false;
	return true;
}
