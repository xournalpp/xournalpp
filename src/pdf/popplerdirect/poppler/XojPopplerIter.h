/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XojPopplerAction.h"
#include "XojPopplerDocument.h"

class GooList;

class XojPopplerIter
{
public:
	XojPopplerIter(XojPopplerDocument doc, const GooList* items);
	virtual ~XojPopplerIter();

public:
	bool next();
	bool isOpen();
	XojPopplerIter* getChildIter();
	XojPopplerAction* getAction();

private:
	static string unicodeToChar(const Unicode* unicode, int len);

private:
	XOJ_TYPE_ATTRIB;

	XojPopplerDocument doc;
	const GooList* items;
	int index;
};
