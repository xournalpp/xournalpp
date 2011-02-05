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
	undoList = g_list_remove_link(undoList, e);
	fireUpdateUndoRedoButtons(undo->getPages());
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
	redoList = g_list_remove_link(redoList, e);
	fireUpdateUndoRedoButtons(redo->getPages());
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
	fireUpdateUndoRedoButtons(action->getPages());
}

void UndoRedoHandler::addUndoActionBefore(UndoAction * action, UndoAction * before) {
	GList * data = g_list_find(undoList, before);
	if (!data) {
		addUndoAction(action);
		return;
	}
	undoList = g_list_insert_before(undoList, data, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
}

bool UndoRedoHandler::removeUndoAction(UndoAction * action) {
	GList * l = g_list_find(this->undoList, action);
	if (l == NULL) {
		return false;
	}

	undoList = g_list_remove_link(undoList, l);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
	return true;
}

String UndoRedoHandler::undoDescription() {
	if (undoList) {
		GList * l = g_list_last(undoList);
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
	if (redoList) {
		GList * l = g_list_last(redoList);
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
	return true;
}

