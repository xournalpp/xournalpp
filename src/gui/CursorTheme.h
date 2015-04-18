/*
 * Xournal++
 *
 * Loads Cursor Themes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <XournalType.h>
#include <String.h>

#include <glib.h>

class CursorTheme
{
public:
	CursorTheme();
	virtual ~CursorTheme();

public:
	bool loadTheme(String name);

public:
	//	GdkCursor * getForPen(ToolSize size, int color);
	//	GdkCursor * getForEraser(ToolSize size, EraserType type);

private:
	XOJ_TYPE_ATTRIB;

	String author;
	String name;
	String description;

};
