/*
 * Xournal++
 *
 * Handles the Cursor
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CURSOR_H__
#define __CURSOR_H__

#include "Control.h"
#include "EditSelection.h"

class Cursor {
public:
	Cursor(Control * control);
	virtual ~Cursor();

	void setCursorBusy(bool busy);
	void updateCursor();

	void setMouseSelectionType(CursorSelectionType selectionType);

	void setMouseDown(bool mouseDown);

	void setInvisible(bool invisible);

private:
	Control * control;
	bool busy;
	CursorSelectionType selectionType;

	bool mouseDown;
	bool invisible;
};

#endif /* __CURSOR_H__ */
