/*
 * Xournal++
 *
 * A layer on a page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __LAYER_H__
#define __LAYER_H__

#include <gtk/gtk.h>

#include "Element.h"
#include "../util/ListIterator.h"
#include "../util/Util.h"

class Layer : public MemoryCheckObject {
public:
	Layer();
	virtual ~Layer();

	void addElement(Element * e);
	void insertElement(Element * e, int pos);
	int indexOf(Element * e);
	int removeElement(Element * e, bool free);

	ListIterator<Element *> elementIterator();

	bool isAnnotated();
private:
	GList * elements;
};

#endif /* __LAYER_H__ */
