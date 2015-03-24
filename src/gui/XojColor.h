/*
 * Xournal++
 *
 * A color to select in the toolbar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __XOJCOLOR_H__
#define __XOJCOLOR_H__

#include <XournalType.h>
#include <glib.h>
#include <StringUtils.h>

class XojColor
{
public:
	XojColor(int color, string name);
	virtual ~XojColor();

public:
	int getColor();
	string getName();

private:
	XOJ_TYPE_ATTRIB;

	int color;
	// the localized name of the color
	string name;
};

#endif /* __XOJCOLOR_H__ */
