/*
 * Xournal++
 *
 * Interface for element containers like selection
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ELEMENTCONTAINER_H__
#define __ELEMENTCONTAINER_H__

#include <ListIterator.h>

class Element;

class ElementContainer {
public:
	virtual ListIterator<Element *> getElements() = 0;
};

#endif /* __ELEMENTCONTAINER_H__ */
