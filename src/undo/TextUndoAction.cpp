#include "TextUndoAction.h"

#include "model/PageRef.h"
#include "model/Layer.h"
#include "model/Text.h"
#include "gui/Redrawable.h"
#include "gui/TextEditor.h"

#include <Rectangle.h>

TextUndoAction::TextUndoAction(PageRef page, Layer* layer,
							   Text* text, string lastText,
							   TextEditor* textEditor) : UndoAction("TextUndoAction")
{
	XOJ_INIT_TYPE(TextUndoAction);

	this->page = page;
	this->layer = layer;
	this->text = text;
	this->lastText = lastText;
	this->textEditor = textEditor;
}

TextUndoAction::~TextUndoAction()
{
	XOJ_RELEASE_TYPE(TextUndoAction);
}

string TextUndoAction::getUndoText()
{
	XOJ_CHECK_TYPE(TextUndoAction);

	return this->lastText;
}

void TextUndoAction::textEditFinished()
{
	XOJ_CHECK_TYPE(TextUndoAction);

	this->textEditor = NULL;
}

string TextUndoAction::getText()
{
	XOJ_CHECK_TYPE(TextUndoAction);

	return _("Text changes");
}

bool TextUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(TextUndoAction);

	double x1 = text->getX();
	double y1 = text->getY();
	double x2 = text->getX() + text->getElementWidth();
	double y2 = text->getY() + text->getElementHeight();

	newText = text->getText();
	text->setText(lastText);
	this->textEditor->setText(lastText);

	x1 = MIN(x1, text->getX());
	y1 = MIN(y1, text->getY());
	x2 = MAX(x2, text->getX() + text->getElementWidth());
	y2 = MAX(y2, text->getY() + text->getElementHeight());

	Rectangle rect(x1, y1, x2 - x1, y2 - y1);
	this->page->fireRectChanged(rect);

	this->undone = true;
	return true;
}

bool TextUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(TextUndoAction);

	double x1 = text->getX();
	double y1 = text->getY();
	double x2 = text->getX() + text->getElementWidth();
	double y2 = text->getY() + text->getElementHeight();

	text->setText(newText);
	this->textEditor->setText(newText);

	x1 = MIN(x1, text->getX());
	y1 = MIN(y1, text->getY());
	x2 = MAX(x2, text->getX() + text->getElementWidth());
	y2 = MAX(y2, text->getY() + text->getElementHeight());

	Rectangle rect(x1, y1, x2 - x1, y2 - y1);
	this->page->fireRectChanged(rect);

	this->undone = false;
	return true;
}
