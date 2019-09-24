/*
 * Xournal++
 *
 * A page reference, should only allocated on the stack
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XojPage.h"

#include <XournalType.h>

class Layer;
class BackgroundImage;

class PageRef
{
public:
	// Todo move constructor
	PageRef();
	PageRef(const PageRef& ref);
	PageRef(XojPage* page);
	virtual ~PageRef();

public:
	bool isValid();

	operator XojPage* ();

	bool operator==(const PageRef& ref);
	void operator=(const PageRef& ref);
	void operator=(XojPage* page);

	XojPage &operator*();
	XojPage *operator->();

	const XojPage &operator*() const;
	const XojPage *operator->() const;

	PageRef clone();

private:
	XojPage* page = nullptr;
};

