/*
 * Xournal++
 *
 * Interface for element containers like selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Element.h"

class Element;

class ElementContainer
{
public:
	virtual ElementVector* getElements() = 0;
	virtual ~ElementContainer() {}
};
