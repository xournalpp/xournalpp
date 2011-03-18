#include "FontUndoAction.h"

#include "../model/Font.h"
#include "../model/Text.h"
#include "../gui/Redrawable.h"

class FontUndoActionEntry {
public:
	FontUndoActionEntry(Text * e, XojFont & oldFont, XojFont & newFont) {
		this->e = e;
		this->oldFont = oldFont;
		this->newFont = newFont;
	}

	Text * e;
	XojFont oldFont;
	XojFont newFont;
};

FontUndoAction::FontUndoAction(XojPage * page, Layer * layer, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

FontUndoAction::~FontUndoAction() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
}

void FontUndoAction::addStroke(Text * e, XojFont & oldFont, XojFont & newFont) {
	this->data = g_list_append(this->data, new FontUndoActionEntry(e, oldFont, newFont));
}

bool FontUndoAction::undo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	FontUndoActionEntry * e = (FontUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;

		// size with old font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());

		e->e->setFont(e->oldFont);

		// size with new font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->rerender(x1, y1, x2, y2);

	return true;
}

bool FontUndoAction::redo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	FontUndoActionEntry * e = (FontUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		FontUndoActionEntry * e = (FontUndoActionEntry *) l->data;

		// size with old font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());

		e->e->setFont(e->newFont);

		// size with new font
		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	view->rerender(x1, y1, x2, y2);

	return true;
}

String FontUndoAction::getText() {
	return _("Change font");
}
