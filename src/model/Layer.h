/*
 * Xournal++
 *
 * A layer on a page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
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
	 * @note Performs a check to determine whether the element is already contained in the Layer
	 */
	void addElement(Element* e);

	/**
	 * Inserts an Element in the specified position of the Layer%s internal list
	 *
	 * @note Performs a check to determine whether the element is already contained in the Layer
	 */
	void insertElement(Element* e, int pos);

	/**
	 * Returns the index of the given Element with respect to the internal list
	 */
	int indexOf(Element* e);

	/**
	 * Removes an Element from the Layer and optionally deletes it
	 */
	int removeElement(Element* e, bool free);

	/**
	 * Returns an iterator over the Element%s contained in this Layer
	 */
	vector<Element*>* getElements();

	/**
	 * Returns whether or not the Layer is empty
	 */
	bool isAnnotated();

	/**
	 * @return true if the layer is visible
	 */
	bool isVisible();

	/**
	 * @param visible true if the layer is visible
	 */
	void setVisible(bool visible);

	/**
	 * Creates a deep copy of this Layer by copying all of the Element%s contained in it
	 */
	Layer* clone();

private:
	vector<Element*> elements;

	bool visible = true;
};
