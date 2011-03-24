/*
 * Xournal++
 *
 * A list with parts
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __PARTLIST_H__
#define __PARTLIST_H__

#include <gtk/gtk.h>
#include "../../util/XournalType.h"

class EraseableStrokePart;

class PartList {
public:
	PartList();
	virtual ~PartList();

public:
	void add(EraseableStrokePart * part);
	PartList * clone();

private:
	XOJ_TYPE_ATTRIB;

	GList * data;

	friend class EraseableStroke;
};

#endif /* __PARTLIST_H__ */
