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
#include <ListIterator.h>
#include <XournalType.h>

class Layer
{
public:
	Layer();
	virtual ~Layer();

public:
	/**
	 * Appends an Element to this Layer
	 *
	 * @note Performs a check to determine whether the element
	 *       is already contained in the Layer
	 */
	void addElement(Element* e);

	/**
	 * Inserts an Element in the specified position of the Layer%s
	 * internal list
	 *
	 * @note Performs a check to determine whether the element
	 *       is already contained in the Layer
	 */
	void insertElement(Element* e, int pos);

	/**
	 * Returns the index of the given Element with respect
	 * to the internal list
	 */
	int indexOf(Element* e);

	/**
	 * Removes an Element from the Layer and optionally deletes it
	 */
	int removeElement(Element* e, bool free);

	/**
	 * Returns an iterator over the Element%s contained
	 * in this Layer
	 */
	ListIterator<Element*> elementIterator();

	/**
	 * Returns whether or not the Layer is empty
	 */
	bool isAnnotated();

	/**
	 * Creates a deep copy of this Layer by copying all
	 * of the Element%s contained in it
	 */
	Layer* clone();

private:
	XOJ_TYPE_ATTRIB;

	GList* elements;

};

#endif /* __LAYER_H__ */
