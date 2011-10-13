/*
 * Xournal++
 *
 * List of unused colors for toolbar customisation
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CUSTOMIZEABLECOLORLIST_H__
#define __CUSTOMIZEABLECOLORLIST_H__

#include <XournalType.h>
#include <glib.h>

class CustomizeableColorList {
public:
	CustomizeableColorList();
	virtual ~CustomizeableColorList();

private:
	XOJ_TYPE_ATTRIB;

	GList * customColors;

public:
	static const int PREDEFINED_COLORS[16];

};

#endif /* __CUSTOMIZEABLECOLORLIST_H__ */
