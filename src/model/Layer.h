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
#include "../util/XournalType.h"

class LayerListener;

class Layer {
public:
	Layer();
	virtual ~Layer();

public:
	void addElement(Element * e);
	void insertElement(Element * e, int pos);
	int indexOf(Element * e);
	int removeElement(Element * e, bool free);

	ListIterator<Element *> elementIterator();

	bool isAnnotated();

public:
	void addListener(LayerListener * listener);
	void removeListener(LayerListener * listener);

private:
	XOJ_TYPE_ATTRIB;

	GList * elements;

	GList * listeners;
};

#endif /* __LAYER_H__ */
