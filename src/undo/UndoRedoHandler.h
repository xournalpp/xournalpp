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
#include "../util/MemoryCheck.h"

class Control;

class UndoRedoListener {
public:
	virtual void undoRedoChanged() = 0;
	virtual void undoRedoPageChanged(XojPage * page) = 0;
};

class UndoRedoHandler: public MemoryCheckObject {
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

private:
	void clearRedo();

private:
	GList * undoList;
	GList * redoList;

	GList * savedUndoList;
	GList * autosavedUndoList;


	GList * listener;

	Control * control;
};

#endif /* __UNDOREDOHANDLER_H__ */
