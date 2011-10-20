/*
 * Xournal++
 *
 * A color to select in the toolbar
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOJCOLOR_H__
#define __XOJCOLOR_H__

#include <XournalType.h>
#include <glib.h>
#include <String.h>

class XojColor {
public:
	XojColor(int color, String name);
	virtual ~XojColor();

public:
	int getColor();
	String getName();

private:
	XOJ_TYPE_ATTRIB;

	int color;
	// the localized name of the color
	String name;
};

#endif /* __XOJCOLOR_H__ */
