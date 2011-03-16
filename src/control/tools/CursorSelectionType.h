/*
 * Xournal++
 *
 * The cursor types for selections
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CURSORSELECTIONTYPE_H__
#define __CURSORSELECTIONTYPE_H__

enum CursorSelectionType {
	CURSOR_SELECTION_NONE = 0,
	//Inside the selection
	CURSOR_SELECTION_MOVE = 1,
	// Edges
	CURSOR_SELECTION_TOP_LEFT,
	CURSOR_SELECTION_TOP_RIGHT,
	CURSOR_SELECTION_BOTTOM_LEFT,
	CURSOR_SELECTION_BOTTOM_RIGHT,
	// Sides
	CURSOR_SELECTION_LEFT,
	CURSOR_SELECTION_RIGHT,
	CURSOR_SELECTION_TOP,
	CURSOR_SELECTION_BOTTOM,
};

#endif /* __CURSORSELECTIONTYPE_H__ */
