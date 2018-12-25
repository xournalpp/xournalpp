#include "UndoRedoHandler.h"

#include "control/Control.h"

#include <config.h>
#include <i18n.h>
#include <XojMsgBox.h>

#include <inttypes.h>

#ifdef UNDO_TRACE

void printAction(UndoAction* action)
{
	if (action)
	{
		g_message("%" PRIu64 " / %s", (uint64_t)action, action->getClassName());
	}
	else
	{
		g_message("(null)");
	}
}

void printUndoList(GList* list)
{
	for (GList* l = list; l != NULL; l = l->next)
	{
		UndoAction* action = (UndoAction*) l->data;
		printAction(action);
	}
}

#endif //UNDO_TRACE

#ifdef UNDO_TRACE
#define PRINTCONTENTS()						\
	g_message("redoList");					\
	printUndoList(this->redoList);			\
	g_message("undoList");					\
	printUndoList(this->undoList);			\
	g_message("savedUndo");					\
	if (this->savedUndo)					\
	{										\
		printAction(this->savedUndo);		\
	}                                                        
#else
#define PRINTCONTENTS()
#endif //UNDO_TRACE

UndoRedoListener::~UndoRedoListener() { }

UndoRedoHandler::UndoRedoHandler(Control* control)
{
	XOJ_INIT_TYPE(UndoRedoHandler);

	this->undoList = NULL;
	this->savedUndo = NULL;
	this->autosavedUndo = NULL;
	this->redoList = NULL;
	this->listener = NULL;
	this->control = control;
}

UndoRedoHandler::~UndoRedoHandler()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	clearContents();

	XOJ_RELEASE_TYPE(UndoRedoHandler);
}

void UndoRedoHandler::clearContents()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	for (GList* l = this->undoList; l != NULL; l = l->next)
	{
		UndoAction* action = (UndoAction*) l->data;

#ifdef UNDO_TRACE
		g_message("clearContents()::Delete UndoAction: %" PRIu64 " / %s", (uint64_t)action, action->getClassName());
#endif //UNDO_TRACE

		delete action;
	}
	g_list_free(this->undoList);
	this->undoList = NULL;

	clearRedo();

	this->savedUndo = NULL;
	this->autosavedUndo = NULL;

	PRINTCONTENTS();
}

void UndoRedoHandler::clearRedo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	for (GList* l = this->redoList; l != NULL; l = l->next)
	{
		UndoAction* action = (UndoAction*) l->data;

#ifdef UNDO_TRACE
		g_message("clearRedo()::Delete UndoAction: %" PRIu64 " / %s", (uint64_t)action, action->getClassName());
#endif

		delete action;
	}
	g_list_free(this->redoList);
	this->redoList = NULL;

	PRINTCONTENTS();
}

void UndoRedoHandler::undo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->undoList)
	{
		return;
	}

	GList* e = g_list_last(this->undoList);
	if (e == NULL)
	{
		g_warning("UndoRedoHandler::undo() e == NULL");
		return;
	}

	UndoAction* undo = (UndoAction*) e->data;

	Document* doc = control->getDocument();
	doc->lock();
	bool undoResult = undo->undo(this->control);
	doc->unlock();

	if (!undoResult)
	{
		string msg = FS(_F("Could not undo \"{1}\"\n" "Something went wrong… Please write a bug report…")
							% undo->getText());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
	}

	this->redoList = g_list_append(this->redoList, undo);
	this->undoList = g_list_delete_link(this->undoList, e);
	fireUpdateUndoRedoButtons(undo->getPages());

	PRINTCONTENTS();
}

void UndoRedoHandler::redo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->redoList)
	{
		return;
	}

	GList* e = g_list_last(this->redoList);
	if (e == NULL)
	{
		g_warning("UndoRedoHandler::redo() e == NULL");
		return;
	}

	UndoAction* redo = (UndoAction*) e->data;

	Document* doc = control->getDocument();
	doc->lock();
	bool redoResult = redo->redo(this->control);
	doc->unlock();

	if (!redoResult)
	{
		string msg = FS(_F("Could not redo \"{1}\"\n" "Something went wrong… Please write a bug report…")
							% redo->getText());
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
	}

	this->undoList = g_list_append(this->undoList, redo);
	this->redoList = g_list_delete_link(this->redoList, e);
	fireUpdateUndoRedoButtons(redo->getPages());

	PRINTCONTENTS();
}

bool UndoRedoHandler::canUndo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	return this->undoList != NULL;
}

bool UndoRedoHandler::canRedo()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	return this->redoList != NULL;
}

/**
 * Adds an undo Action to the list, or if NULL does nothing
 */
void UndoRedoHandler::addUndoAction(UndoAction* action)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (action == NULL)
	{
		return;
	}

	this->undoList = g_list_append(this->undoList, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());

	PRINTCONTENTS();
}

void UndoRedoHandler::addUndoActionBefore(UndoAction* action, UndoAction* before)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	GList* data = g_list_find(this->undoList, before);
	if (!data)
	{
		addUndoAction(action);
		return;
	}
	this->undoList = g_list_insert_before(this->undoList, data, action);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());

	PRINTCONTENTS();
}

bool UndoRedoHandler::removeUndoAction(UndoAction* action)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	GList* l = g_list_find(this->undoList, action);
	if (l == NULL)
	{
		return false;
	}

	undoList = g_list_delete_link(undoList, l);
	clearRedo();
	fireUpdateUndoRedoButtons(action->getPages());
	return true;
}

string UndoRedoHandler::undoDescription()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList)
	{
		GList* l = g_list_last(this->undoList);
		UndoAction* a = (UndoAction*) l->data;
		if (!a->getText().empty())
		{
			string txt = _("Undo: ");
			txt += a->getText();
			return txt;
		}
	}
	return _("Undo");
}

string UndoRedoHandler::redoDescription()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->redoList)
	{
		GList* l = g_list_last(this->redoList);
		UndoAction* a = (UndoAction*) l->data;
		if (!a->getText().empty())
		{
			string txt = _("Redo: ");
			txt += a->getText();

			return txt;
		}
	}
	return _("Redo");
}

void UndoRedoHandler::fireUpdateUndoRedoButtons(vector<PageRef> pages)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	for (GList* l = this->listener; l != NULL; l = l->next)
	{
		((UndoRedoListener*) l->data)->undoRedoChanged();
	}

	for (PageRef page : pages)
	{
		if (!page.isValid())
		{
			continue;
		}

		for (GList* l = this->listener; l != NULL; l = l->next)
		{
			((UndoRedoListener*) l->data)->undoRedoPageChanged(page);
		}
	}
}

void UndoRedoHandler::addUndoRedoListener(UndoRedoListener* listener)
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	this->listener = g_list_append(this->listener, listener);
}

bool UndoRedoHandler::isChanged()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->undoList) return this->savedUndo;

	return this->savedUndo != g_list_last(this->undoList)->data;
}

bool UndoRedoHandler::isChangedAutosave()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (!this->undoList) return this->autosavedUndo;

	return this->autosavedUndo != g_list_last(this->undoList)->data;
}

void UndoRedoHandler::documentAutosaved()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList)
	{
		this->autosavedUndo = (UndoAction*) g_list_last(this->undoList)->data;
	}
	else
	{
		this->autosavedUndo = NULL;
	}
}

void UndoRedoHandler::documentSaved()
{
	XOJ_CHECK_TYPE(UndoRedoHandler);

	if (this->undoList)
	{
		this->savedUndo = (UndoAction*) g_list_last(this->undoList)->data;
	}
	else
	{
		this->savedUndo = NULL;
	}
}

const char* UndoRedoHandler::getUndoStackTopTypeName()
{
	GList* e = g_list_last(this->undoList);
	if (e == NULL)
	{
		return NULL;
	}

	return ((UndoAction*) e->data)->getClassName();
}

const char* UndoRedoHandler::getRedoStackTopTypeName()
{
	GList* e = g_list_last(this->redoList);
	if (e == NULL)
	{
		return NULL;
	}

	return ((UndoAction*) e->data)->getClassName();
}
