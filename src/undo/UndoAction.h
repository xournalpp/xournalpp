/*
 * Xournal++
 *
 * Abstract undo action
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __UNDOACTION_H__
#define __UNDOACTION_H__

#include "../util/String.h"

#include <config.h>
#include <glib/gi18n-lib.h>

class Control;
class XojPage;

class UndoAction {
public:
	UndoAction();

	virtual bool undo(Control * control) = 0;
	virtual bool redo(Control * control) = 0;

	virtual String getText() = 0;

	/**
	 * Get the affected pages, the Array is terminated with NULL and should be freed with delete[]
	 */
	virtual XojPage ** getPages();
protected:
	XojPage * page;
	bool undone;
};

#endif /* __UNDOACTION_H__ */
