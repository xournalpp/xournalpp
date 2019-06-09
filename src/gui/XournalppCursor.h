/*
 * Xournal++
 *
 * Handles the XournalppCursor
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
#include <gui/inputdevices/InputEvents.h>

class Control;

class XournalppCursor
{
public:
	XournalppCursor(Control* control);
	virtual ~XournalppCursor();

	void setCursorBusy(bool busy);
	void updateCursor();

	void setMouseSelectionType(CursorSelectionType selectionType);

	void setMouseDown(bool mouseDown);
	void setInvisible(bool invisible);
	void setInsidePage(bool insidePage);
	void setStockCursor(GdkCursorType type);
	void activateDrawDirCursor(bool enable, bool shift=false, bool ctrl=false);
	void setInputDeviceClass(InputDeviceClass inputDevice);
	


private:
	GdkCursor* getPenCursor();

	GdkCursor* getEraserCursor();
	GdkCursor* getHighlighterCursor();

	GdkCursor* createHighlighterOrPenCursor(int size, double alpha);
	GdkCursor* createCustomDrawDirCursor(int size, bool shift, bool ctrl);
	
	void doDrawDirCursor();

	
private:
	XOJ_TYPE_ATTRIB;

	InputDeviceClass inputDevice = INPUT_DEVICE_MOUSE;

	Control* control = NULL;
	bool busy = false;
	bool insidePage = false;
	CursorSelectionType selectionType = CURSOR_SELECTION_NONE;

	bool mouseDown = false;
	bool invisible = false;
	
	// One shot drawDir custom cursor -drawn instead of pen/stylus then cleared.
	bool drawDirActive = false;
	bool drawDirShift = false;
	bool drawDirCtrl = false;	
	
	
	//combination to avoid making same cursor
	void* lastCustomCursorAddress = NULL;	//for comparison only
	int lastCustomCursorType = 0;	//our own id
};
