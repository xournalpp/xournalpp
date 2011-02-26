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

#include "../control/tools/CursorSelectionType.h"
#include <gtk/gtk.h>

class Control;

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
	GdkCursor * getPenCursor();

private:
	Control * control;
	bool busy;
	CursorSelectionType selectionType;

	bool mouseDown;
	bool invisible;
};

#endif /* __CURSOR_H__ */
