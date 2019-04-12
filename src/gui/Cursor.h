/*
 * Xournal++
 *
 * Handles the Cursor
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/tools/CursorSelectionType.h"
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
	void setTempCursor(GdkCursorType type);
	void setDrawDirCursor(bool shift, bool ctrl);
	


private:
	GdkCursor* getPenCursor();

	GdkCursor* getEraserCursor();
	GdkCursor* getHighlighterCursor();

	GdkCursor* createHighlighterOrPenCursor(int size, double alpha);
	GdkCursor* createCustomDrawDirCursor(int size, bool shift, bool ctrl);
	
private:
	XOJ_TYPE_ATTRIB;


	Control* control = NULL;
	bool busy = false;
	bool insidePage = false;
	CursorSelectionType selectionType = CURSOR_SELECTION_NONE;

	bool mouseDown = false;
	bool invisible = false;
};
