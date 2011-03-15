#include <stdio.h>
#include "UndoRedoHandler.h"
#include "../control/Control.h"

#include <config.h>
#include <glib/gi18n-lib.h>

// TODO: LOW PRIO: add memory limit

UndoRedoHandler::UndoRedoHandler(Control * control) {
	this->undoList = NULL;
	this->savedUndoList = NULL;
	this->autosavedUndoList = NULL;
	this->redoList = NULL;
	this->listener = NULL;
	this->control = control;
}

UndoRedoHandler::~UndoRedoHandler() {
	clearContents();
}

void UndoRedoHandler::clearContents() {
	for (GList * l = this->undoList; l != NULL; l = l->next) {
		UndoAction * action = (UndoAction *) l->data;
		delete action;
	}
	g_list_free(this->undoList);
	this->undoList = NULL;

	clearRedo();
}

void UndoRedoHandler::clearRedo() {
	for (GList * l = this->redoList; l != NULL; l = l->next) {
		UndoAction * action = (UndoAction *) l->data;
		delete action;
	}
	g_list_free(this->redoList);
	this->redoList = NULL;
}

void UndoRedoHandler::undo() {
	if (!this->undoList) {
		return;
	}

	GList * e = g_list_last(this->undoList);
	if (e == NULL) {
		g_warning("UndoRedoHandler::undo() e == NULL");
		return;
	}

	UndoAction * undo = (UndoAction *) e->data;

	Document * doc = control->getDocument();
	doc->lock();
	bool undoResult = undo->undo(this->control);
	doc->unlock();

	if (!undoResult) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *control->getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Could not undo '%s'\nSomething went wrong... Please write a bug report..."), undo->getText().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	this->redoList = g_list_append(this->redoList, undo);
	this->undoList = g_list_delete_link(this->undoList, e);
	fireUpdateUndoRedoButtons(undo->getPages());
}

void UndoRedoHandler::redo() {
	if (!this->redoList) {
		return;
	}

	GList * e = g_list_last(this->redoList);
	if (e == NULL) {
		g_warning("UndoRedoHandler::redo() e == NULL");
		return;
	}

	UndoAction * redo = (UndoAction *) e->data;

	Document * doc = control->getDocument();
	doc->lock();
	bool redoResult = redo->redo(this->control);
	doc->unlock();

	if (!redoResult) {
		GtkWidget * dialog = gtk_message_dialog_new((GtkWindow *) *control->getWindow(), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				_("Could not redo '%s'\nSomething went wrong... Please write a bug report..."), redo->getText().c_str());
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	this->undoList = g_list_append(this->undoList, redo);
	this->redoList = g_list_delete_link(this->redoList, e);
	fireUpdateUndoRedoButtons(redo->getPages());
}

bool UndoRedoHandler::canUndo() {
	return this->undoList != NULL;
}

bool UndoRedoHandler::canRedo() {
	return this->redoList != NULL;
}

void UndoRedoHandler::addUndoAction(UndoAction * action) {
	this->undoList = g_list_append(this->undoList, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
}

void UndoRedoHandler::addUndoActionBefore(UndoAction * action, UndoAction * before) {
	GList * data = g_list_find(this->undoList, before);
	if (!data) {
		addUndoAction(action);
		return;
	}
	this->undoList = g_list_insert_before(this->undoList, data, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
}

bool UndoRedoHandler::removeUndoAction(UndoAction * action) {
	GList * l = g_list_find(this->undoList, action);
	if (l == NULL) {
		return false;
	}

	undoList = g_list_delete_link(undoList, l);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
	return true;
}

String UndoRedoHandler::undoDescription() {
	if (this->undoList) {
		GList * l = g_list_last(this->undoList);
		UndoAction * a = (UndoAction *) l->data;
		if (!a->getText().isEmpty()) {
			String txt = _("Undo: ");
			txt += a->getText();
			return txt;
		}
	}
	return _("Undo");
}

String UndoRedoHandler::redoDescription() {
	if (this->redoList) {
		GList * l = g_list_last(this->redoList);
		UndoAction * a = (UndoAction *) l->data;
		if (!a->getText().isEmpty()) {
			String txt = _("Redo: ");
			txt += a->getText();

			return txt;
		}
	}
	return _("Redo");
}

void UndoRedoHandler::fireUpdateUndoRedoButtons(XojPage ** pages) {
	for (GList * l = this->listener; l != NULL; l = l->next) {
		((UndoRedoListener *) l->data)->undoRedoChanged();
	}

	for (int i = 0; pages[i]; i++) {
		for (GList * l = this->listener; l != NULL; l = l->next) {
			((UndoRedoListener *) l->data)->undoRedoPageChanged(pages[i]);
		}
	}
	delete[] pages;
}

void UndoRedoHandler::addUndoRedoListener(UndoRedoListener * listener) {
	this->listener = g_list_append(this->listener, listener);
}

bool UndoRedoHandler::isChanged() {
	return this->savedUndoList != g_list_last(this->undoList);
}

bool UndoRedoHandler::isChangedAutosave() {
	return this->autosavedUndoList != g_list_last(this->undoList);
}

void UndoRedoHandler::documentAutosaved() {
	this->autosavedUndoList = g_list_last(this->undoList);
}

void UndoRedoHandler::documentSaved() {
	this->savedUndoList = g_list_last(this->undoList);
}
