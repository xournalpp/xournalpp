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

// TODO: AA: type check

#ifndef __PARTLIST_H__
#define __PARTLIST_H__

#include <gtk/gtk.h>

class EraseableStrokePart;

class PartList {
public:
	PartList();
	virtual ~PartList();

public:
	void add(EraseableStrokePart * part);
	PartList * clone();

private:
	GList * data;

	friend class EraseableStroke;
};

#endif /* __PARTLIST_H__ */
