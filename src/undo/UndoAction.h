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

#ifndef __UNDOACTION_H__
#define __UNDOACTION_H__

#include "../util/String.h"
#include "../model/PageRef.h"

#include <config.h>
#include <glib/gi18n-lib.h>

class Control;
class XojPage;

class UndoAction {
public:
	UndoAction(const char * className);
	virtual ~UndoAction();

public:
	virtual bool undo(Control * control) = 0;
	virtual bool redo(Control * control) = 0;

	virtual String getText() = 0;

	/**
	 * Get the affected pages, the Array is terminated with NULL and should be freed with delete[]
	 */
	virtual XojPage ** getPages();

	const char * getClassName() const;

protected:
	XOJ_TYPE_ATTRIB;

	// This is only for debugging / Testing purpose
	const char * className;

	PageRef page;
	bool undone;
};

#endif /* __UNDOACTION_H__ */
