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
#include <XournalType.h>

#include <gtk/gtk.h>

class Control;

class Cursor
{
public:
	Cursor(Control* control);
	virtual ~Cursor();

	void setCursorBusy(bool busy);
	void updateCursor();

	void setMouseSelectionType(CursorSelectionType selectionType);

	void setMouseDown(bool mouseDown);
	void setInvisible(bool invisible);
	void setInsidePage(bool insidePage);

private:
	GdkCursor* getPenCursor();

	GdkCursor* eraserCursor();
	GdkCursor* highlighterCursor();

private:
	XOJ_TYPE_ATTRIB;


	Control* control;
	bool busy;
	bool insidePage;
	CursorSelectionType selectionType;

	bool mouseDown;
	bool invisible;
};

#endif /* __CURSOR_H__ */
