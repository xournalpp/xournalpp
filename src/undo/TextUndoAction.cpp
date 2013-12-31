#include "TextUndoAction.h"

#include "../model/PageRef.h"
#include "../model/Layer.h"
#include "../model/Text.h"
#include "../gui/Redrawable.h"
#include "../gui/TextEditor.h"

TextUndoAction::TextUndoAction(PageRef page, Layer* layer, Text* text,
                               String lastText, Redrawable* view,
                               TextEditor* textEditor) : UndoAction("TextUndoAction")
{
	XOJ_INIT_TYPE(TextUndoAction);

	this->page = page;
	this->layer = layer;
	this->text = text;
	this->lastText = lastText;
	this->view = view;
	this->textEditor = textEditor;
}

TextUndoAction::~TextUndoAction()
{
	XOJ_RELEASE_TYPE(TextUndoAction);
}

String TextUndoAction::getUndoText()
{
	XOJ_CHECK_TYPE(TextUndoAction);

	return this->lastText;
}

void TextUndoAction::textEditFinished()
{
	XOJ_CHECK_TYPE(TextUndoAction);

	this->textEditor = NULL;
}

String TextUndoAction::getText()
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

	view->rerenderArea(x1, y1, x2, y2);

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

	view->rerenderArea(x1, y1, x2, y2);

	this->undone = false;
	return true;
}
