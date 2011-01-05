#include <stdio.h>
#include "UndoRedoHandler.h"
#include "../gettext.h"
#include "../control/Control.h"

UndoRedoHandler::UndoRedoHandler(Control * control) {
	undoList = NULL;
	redoList = NULL;
	listener = NULL;
	this->control = control;
}

UndoRedoHandler::~UndoRedoHandler() {
	clearContents();
}

void UndoRedoHandler::clearContents() {
	for (GList * l = undoList; l != NULL; l = l->next) {
		UndoAction * action = (UndoAction *) l->data;
		delete action;
	}
	g_list_free(undoList);
	undoList = NULL;

	clearRedo();
}

void UndoRedoHandler::clearRedo() {
	for (GList * l = redoList; l != NULL; l = l->next) {
		UndoAction * action = (UndoAction *) l->data;
		delete action;
	}
	g_list_free(redoList);
	redoList = NULL;
}

void UndoRedoHandler::undo() {
	if (!undoList) {
		return;
	}

	GList * e = g_list_last(undoList);
	if (e == NULL) {
		g_warning("UndoRedoHandler::undo() e == NULL");
		return;
	}

	UndoAction * undo = (UndoAction *) e->data;
	if (!undo->undo(this->control)) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *control->getWindow(),
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Could not undo '%s'\nSomething went wrong... Please write a bug report..."),
				undo->getText().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	redoList = g_list_append(redoList, undo);
	undoList = g_list_remove(undoList, e->data);
	fireUpdateUndoRedoButtons(undo->getPage());
}

void UndoRedoHandler::redo() {
	if (!redoList) {
		return;
	}

	GList * e = g_list_last(redoList);
	if (e == NULL) {
		g_warning("UndoRedoHandler::redo() e == NULL");
		return;
	}

	UndoAction * redo = (UndoAction *) e->data;
	if (!redo->redo(this->control)) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *control->getWindow(),
				GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Could not redo '%s'\nSomething went wrong... Please write a bug report..."),
				redo->getText().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	undoList = g_list_append(undoList, redo);
	redoList = g_list_remove(redoList, e->data);
	fireUpdateUndoRedoButtons(redo->getPage());
}

bool UndoRedoHandler::canUndo() {
	return undoList != NULL;
}

bool UndoRedoHandler::canRedo() {
	return redoList != NULL;
}

void UndoRedoHandler::addUndoAction(UndoAction * action) {
	undoList = g_list_append(undoList, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPage());
}

void UndoRedoHandler::addUndoActionBefore(UndoAction * action, UndoAction * before) {
	GList * data = g_list_find(undoList, before);
	if (!data) {
		addUndoAction(action);
		return;
	}
	undoList = g_list_insert_before(undoList, data, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPage());
}

bool UndoRedoHandler::removeUndoAction(UndoAction * action) {
	GList * l = g_list_find(this->undoList, action);
	if (l == NULL) {
		return false;
	}

	undoList = g_list_remove_link(undoList, l);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPage());
	return true;
}

String UndoRedoHandler::undoDescription() {
	if (undoList) {
		GList * l = g_list_last(undoList);
		UndoAction * a = (UndoAction *) undoList->data;
		if (!a->getText().isEmpty()) {
			String txt = _("Undo: ");
			txt += a->getText();
			return txt;
		}
	}
	return _("Undo");
}

String UndoRedoHandler::redoDescription() {
	if (redoList) {
		GList * l = g_list_last(redoList);
		UndoAction * a = (UndoAction *) redoList->data;
		if (!a->getText().isEmpty()) {
			String txt = _("Redo: ");
			txt += a->getText();
			return txt;
		}
	}
	return _("Redo");
}

void UndoRedoHandler::fireUpdateUndoRedoButtons(XojPage * page) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		((UndoRedoListener *) l->data)->undoRedoChanged();
	}

	if (page) {
		for (GList * l = this->listener; l != NULL; l = l->next) {
			((UndoRedoListener *) l->data)->undoRedoPageChanged(page);
		}
	}
}

void UndoRedoHandler::addUndoRedoListener(UndoRedoListener * listener) {
	this->listener = g_list_append(this->listener, listener);
}

bool UndoRedoHandler::isChanged() {
	return true;
}

//////////////////////////////////////////////////////////////

UndoAction::UndoAction() {
	this->undone = false;
}

XojPage * UndoAction::getPage() {
	return page;
}

//////////////////////////////////////////////////////////////

InsertUndoAction::InsertUndoAction(XojPage * page, Layer * layer, Element * element, PageView * view) {
	this->page = page;
	this->layer = layer;
	this->element = element;
	this->view = view;
}

InsertUndoAction::~InsertUndoAction() {
	if (undone) {
		// Insert was undone, so this is not needed anymore
		delete this->element;
	}
}

String InsertUndoAction::getText() {
	if (element->getType() == ELEMENT_STROKE) {
		return _("Draw stroke");
	} else if (element->getType() == ELEMENT_TEXT) {
		return _("Write text");
	} else {
		return NULL;
	}
}

bool InsertUndoAction::undo(Control * control) {
	this->layer->removeElement(this->element, false);

	view->repaint(element->getX(), element->getY(), element->getElementWidth(), element->getElementHeight());

	this->undone = true;

	return true;
}

bool InsertUndoAction::redo(Control * control) {
	this->layer->addElement(this->element);

	view->repaint(element->getX(), element->getY(), element->getElementWidth(), element->getElementHeight());

	this->undone = false;

	return true;
}

//////////////////////////////////////////////////////////////
TextUndoAction::TextUndoAction(XojPage * page, Layer * layer, Text * text, String lastText, PageView * view,
		TextEditor * textEditor) {
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

	view->repaint(x1, y1, x2 - x1, y2 - y1);

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

	view->repaint(x1, y1, x2 - x1, y2 - y1);

	this->undone = false;
	return true;
}

//////////////////////////////////////////////////////////////

InsertLayerUndoAction::InsertLayerUndoAction(XojPage * page, Layer * layer) {
	this->page = page;
	this->layer = layer;
}

InsertLayerUndoAction::~InsertLayerUndoAction() {
	if (undone) {
		// The layer was undone, also deleted
		delete this->layer;
	}
}

String InsertLayerUndoAction::getText() {
	return "Insert layer";
}

bool InsertLayerUndoAction::undo(Control * control) {
	this->page->removeLayer(this->layer);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = true;

	return true;
}

bool InsertLayerUndoAction::redo(Control * control) {
	this->page->addLayer(this->layer);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = false;

	return true;
}
//////////////////////////////////////////////////////////////

RemoveLayerUndoAction::RemoveLayerUndoAction(XojPage * page, Layer * layer, int layerPos) {
	this->page = page;
	this->layer = layer;
	this->layerPos = layerPos;
}

RemoveLayerUndoAction::~RemoveLayerUndoAction() {
	if (!undone) {
		// The layer was NOT undone, also NOT restored
		delete this->layer;
	}
}

String RemoveLayerUndoAction::getText() {
	return "Delete layer";
}

bool RemoveLayerUndoAction::undo(Control * control) {
	this->page->insertLayer(this->layer, layerPos);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = true;

	return true;
}

bool RemoveLayerUndoAction::redo(Control * control) {
	this->page->removeLayer(this->layer);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = false;

	return true;
}

//////////////////////////////////////////////////////////////

template<class T>
class PageLayerPosEntry {
public:
	PageLayerPosEntry(Layer * layer, T * element, int pos) {
		this->element = element;
		this->pos = pos;
		this->layer = layer;
	}

	Layer * layer;
	T * element;
	int pos;

	static int cmp(PageLayerPosEntry<T> * a, PageLayerPosEntry<T> * b) {
		return a->pos - b->pos;
	}
};

//////////////////////////////////////////////////////////////

DeleteUndoAction::DeleteUndoAction(XojPage * page, PageView * view, bool eraser) {
	this->page = page;
	this->view = view;
	this->eraser = eraser;
	this->elements = NULL;
}

DeleteUndoAction::~DeleteUndoAction() {
	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		if (!undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->elements);
}

void DeleteUndoAction::addElement(Layer * layer, Element * e, int pos) {
	this->elements = g_list_insert_sorted(this->elements, new PageLayerPosEntry<Element> (layer, e, pos),
			(GCompareFunc) PageLayerPosEntry<Element>::cmp);
}

bool DeleteUndoAction::undo(Control * control) {
	if (this->elements == NULL) {
		g_warning("Could not undo DeleteUndoAction, there is nothing to undo");

		this->undone = true;
		return false;
	}

	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	return true;
}

bool DeleteUndoAction::redo(Control * control) {
	if (this->elements == NULL) {
		g_warning("Could not redo DeleteUndoAction, there is nothing to redo");

		this->undone = false;
		return false;
	}

	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->removeElement(e->element, false);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	this->undone = false;

	return true;
}

String DeleteUndoAction::getText() {
	String text;

	if (eraser) {
		text = _("Erase Stroke");
	} else {
		text = _("Delete");

		if (this->elements != NULL) {
			ElementType type = ((PageLayerPosEntry<Element>*) this->elements->data)->element->getType();

			for (GList * l = this->elements->next; l != NULL; l = l->next) {
				PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
				if (type != e->element->getType()) {
					text += _(" elements");
					return text;
				}
			}

			if (type == ELEMENT_STROKE) {
				text += _(" stroke");
			} else if (type == ELEMENT_TEXT) {
				text += _(" text");
			} else if (type == ELEMENT_IMAGE) {
				text += _(" image");
			}
		}
	}
	return text;
}

//////////////////////////////////////////////////////////////

RecognizerUndoAction::RecognizerUndoAction(XojPage * page, PageView * view, Layer * layer, Element * original,
		Element * recognized) {
	this->page = page;
	this->view = view;
	this->layer = layer;
	this->original = original;
	this->recognized = recognized;
}

RecognizerUndoAction::~RecognizerUndoAction() {
	if (undone) {
		delete original;
	} else {
		delete recognized;
	}
	original = NULL;
	recognized = NULL;
}

bool RecognizerUndoAction::undo(Control * control) {
	int pos = this->layer->removeElement(this->recognized, false);
	this->layer->insertElement(this->original, pos);

	this->view->repaint();

	undone = true;
	return true;
}

bool RecognizerUndoAction::redo(Control * control) {
	int pos = this->layer->removeElement(this->original, false);
	this->layer->insertElement(this->recognized, pos);

	this->view->repaint();

	undone = false;
	return true;
}

String RecognizerUndoAction::getText() {
	return _("Stroke recognizer");
}

//////////////////////////////////////////////////////////////

InsertDeletePageUndoAction::InsertDeletePageUndoAction(XojPage * page, int pagePos, bool inserted) {
	this->inserted = inserted;
	this->page = page;
	this->pagePos = pagePos;
	page->reference();
}

InsertDeletePageUndoAction::~InsertDeletePageUndoAction() {
	page->unreference();
}

bool InsertDeletePageUndoAction::undo(Control * control) {
	if (this->inserted) {
		return deletePage(control);
	} else {
		return insertPage(control);
	}
}

bool InsertDeletePageUndoAction::redo(Control * control) {
	if (this->inserted) {
		return insertPage(control);
	} else {
		return deletePage(control);
	}
}

bool InsertDeletePageUndoAction::insertPage(Control * control) {
	Document * doc = control->getDocument();

	doc->insertPage(this->page, this->pagePos);
	control->firePageInserted(this->pagePos);

	control->getCursor()->updateCursor();

	control->scrollToPage(this->pagePos);

	control->updateDeletePageButton();

	return true;
}

bool InsertDeletePageUndoAction::deletePage(Control * control) {
	Document * doc = control->getDocument();

	int pNr = doc->indexOf(page);
	if (pNr == -1) {
		// this should not happen
		return false;
	}

	// first send event, then delete page...
	control->firePageDeleted(pNr);
	doc->deletePage(pNr);

	control->updateDeletePageButton();

	return true;
}

String InsertDeletePageUndoAction::getText() {
	if (this->inserted) {
		return _("Page inserted");
	} else {
		return _("Page deleted");
	}
}

//////////////////////////////////////////////////////////////

PageBackgroundChangedUndoAction::PageBackgroundChangedUndoAction(XojPage * page, BackgroundType origType,
		int origPdfPage, BackgroundImage origBackgroundImage, double origW, double origH) {
	this->page = page;
	this->origType = origType;
	this->origPdfPage = this->origPdfPage;
	this->origBackgroundImage = origBackgroundImage;
	this->origW = origW;
	this->origH = origH;
}
PageBackgroundChangedUndoAction::~PageBackgroundChangedUndoAction() {
}

bool PageBackgroundChangedUndoAction::undo(Control * control) {
	this->newType = this->page->getBackgroundType();
	this->newPdfPage = this->page->getPdfPageNr();
	this->newBackgroundImage = this->page->backgroundImage;
	this->newW = this->page->getWidth();
	this->newH = this->page->getHeight();

	int pageNr = control->getDocument()->indexOf(this->page);
	if (pageNr == -1) {
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH) {
		this->page->setSize(this->origW, this->origH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->origType);
	if (this->origType == BACKGROUND_TYPE_PDF) {
		this->page->setBackgroundPdfPageNr(this->origPdfPage);
	} else if (this->origType == BACKGROUND_TYPE_IMAGE) {
		this->page->backgroundImage = this->origBackgroundImage;
	}

	control->firePageChanged(pageNr);

	return true;
}

bool PageBackgroundChangedUndoAction::redo(Control * control) {
	int pageNr = control->getDocument()->indexOf(this->page);
	if (pageNr == -1) {
		return false;
	}

	if (this->newW != this->origW || this->newH != this->origH) {
		this->page->setSize(this->newW, this->newH);
		control->firePageSizeChanged(pageNr);
	}

	this->page->setBackgroundType(this->newType);
	if (this->newType == BACKGROUND_TYPE_PDF) {
		this->page->setBackgroundPdfPageNr(this->newPdfPage);
	} else if (this->newType == BACKGROUND_TYPE_IMAGE) {
		this->page->backgroundImage = this->newBackgroundImage;
	}

	control->firePageChanged(pageNr);

	return true;

}

String PageBackgroundChangedUndoAction::getText() {
	return _("Page background changed");
}

//////////////////////////////////////////////////////////////

EraseUndoAction::EraseUndoAction(XojPage * page, PageView * view) {
	this->page = page;
	this->view = view;
	this->edited = NULL;
	this->original = NULL;
}

EraseUndoAction::~EraseUndoAction() {
	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (!undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->original);

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->edited);
}

bool EraseUndoAction::undo(Control * control) {
	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->removeElement(e->element, false);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	this->view->repaint();

	this->undone = true;
	return true;
}

bool EraseUndoAction::redo(Control * control) {
	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->removeElement(e->element, false);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	this->view->repaint();

	this->undone = false;
	return true;

}

void EraseUndoAction::addOriginal(Layer * layer, Stroke * element, int pos) {
	this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::addEdited(Layer * layer, Stroke * element, int pos) {
	this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::cleanup() {
	for (GList * l = this->edited; l != NULL;) {
		PageLayerPosEntry<Stroke> * p = (PageLayerPosEntry<Stroke> *) l->data;
		if (p->element->getPointCount() == 0) {
			GList * del = l;
			l = l->next;
			this->edited = g_list_remove_link(this->edited, del);
			continue;
		}

		l = l->next;
	}
}

String EraseUndoAction::getText() {
	return _("Erase stroke");
}
