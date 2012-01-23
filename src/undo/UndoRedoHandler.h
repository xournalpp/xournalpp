/*
 * Xournal++
 *
 * Handles Undo and Redo
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __UNDOREDOHANDLER_H__
#define __UNDOREDOHANDLER_H__

#include "UndoAction.h"
#include <XournalType.h>

class Control;

class UndoRedoListener {
public:
	virtual void undoRedoChanged() = 0;
	virtual void undoRedoPageChanged(PageRef page) = 0;
};

class UndoRedoHandler {
public:
	UndoRedoHandler(Control * control);
	virtual ~UndoRedoHandler();

	void undo();
	void redo();

	bool canUndo();
	bool canRedo();

	void addUndoAction(UndoAction * action);
	void addUndoActionBefore(UndoAction * action, UndoAction * before);
	bool removeUndoAction(UndoAction * action);

	String undoDescription();
	String redoDescription();

	void clearContents();

	void fireUpdateUndoRedoButtons(XojPage ** pages);
	void addUndoRedoListener(UndoRedoListener * listener);

	bool isChanged();
	bool isChangedAutosave();
	void documentAutosaved();
	void documentSaved();

	const char * getUndoStackTopTypeName();
	const char * getRedoStackTopTypeName();

private:
	void clearRedo();

private:
	XOJ_TYPE_ATTRIB;

	GList * undoList;
	GList * redoList;

	GList * savedUndoList;
	GList * autosavedUndoList;


	GList * listener;

	Control * control;
};

#endif /* __UNDOREDOHANDLER_H__ */

