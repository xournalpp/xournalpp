/*
 * Xournal++
 *
 * Handles Undo and Redo
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Control;

class UndoRedoListener
{
public:
	virtual void undoRedoChanged() = 0;
	virtual void undoRedoPageChanged(PageRef page) = 0;
};

class UndoRedoHandler
{
public:
	UndoRedoHandler(Control* control);
	virtual ~UndoRedoHandler();

	void undo();
	void redo();

	bool canUndo();
	bool canRedo();

	void addUndoAction(UndoAction* action);
	void addUndoActionBefore(UndoAction* action, UndoAction* before);
	bool removeUndoAction(UndoAction* action);

	string undoDescription();
	string redoDescription();

	void clearContents();

	void fireUpdateUndoRedoButtons(XojPage** pages);
	void addUndoRedoListener(UndoRedoListener* listener);

	bool isChanged();
	bool isChangedAutosave();
	void documentAutosaved();
	void documentSaved();

	const char* getUndoStackTopTypeName();
	const char* getRedoStackTopTypeName();

private:
	void clearRedo();

private:
	XOJ_TYPE_ATTRIB;

	GList* undoList;
	GList* redoList;

	UndoAction* savedUndo;
	UndoAction* autosavedUndo;


	GList* listener;

	Control* control;
};
