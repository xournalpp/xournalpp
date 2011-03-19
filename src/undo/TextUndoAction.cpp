#include "TextUndoAction.h"

#include "../model/Page.h"
#include "../model/Layer.h"
#include "../model/Text.h"
#include "../gui/Redrawable.h"
#include "../gui/TextEditor.h"

TextUndoAction::TextUndoAction(XojPage * page, Layer * layer, Text * text, String lastText, Redrawable * view, TextEditor * textEditor) {
	this->page = page;
	this->layer = layer;
	this->text = text;
	this->lastText = lastText;
	this->view = view;
	this->textEditor = textEditor;
}

TextUndoAction::~TextUndoAction() {
}

String TextUndoAction::getUndoText() {
	return lastText;
}

void TextUndoAction::textEditFinished() {
	this->textEditor = NULL;
}

String TextUndoAction::getText() {
	return "Text changes";
}

bool TextUndoAction::undo(Control * control) {
	double x1 = text->getX();
	double y1 = text->getY();
	double x2 = text->getX() + text->getElementWidth();
	double y2 = text->getY() + text->getElementHeight();

	newText = text->getText();
	text->setText(lastText);
	this->textEditor->setText(lastText);

	x1 = MIN(x1, text->getX());
	y1 = MIN(y1,text->getY());
	x2 = MAX(x2, text->getX() + text->getElementWidth());
	y2 = MAX(y2, text->getY() + text->getElementHeight());

	view->rerenderArea(x1, y1, x2, y2);

	this->undone = true;
	return true;
}

bool TextUndoAction::redo(Control * control) {
	double x1 = text->getX();
	double y1 = text->getY();
	double x2 = text->getX() + text->getElementWidth();
	double y2 = text->getY() + text->getElementHeight();

	text->setText(newText);
	this->textEditor->setText(newText);

	x1 = MIN(x1, text->getX());
	y1 = MIN(y1,text->getY());
	x2 = MAX(x2, text->getX() + text->getElementWidth());
	y2 = MAX(y2, text->getY() + text->getElementHeight());

	view->rerenderArea(x1, y1, x2, y2);

	this->undone = false;
	return true;
}
