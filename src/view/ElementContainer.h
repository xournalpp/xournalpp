/*
 * Xournal++
 *
 * Interface for element containers like selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __ELEMENTCONTAINER_H__
#define __ELEMENTCONTAINER_H__

#include "../model/Element.h"

class Element;

class ElementContainer
{
public:
	virtual ElementVector* getElements() = 0;
};

#endif /* __ELEMENTCONTAINER_H__ */
