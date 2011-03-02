/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOJPOPPLERITER_H__
#define __XOJPOPPLERITER_H__

#include "XojPopplerDocument.h"
#include "XojPopplerAction.h"

class GooList;

class XojPopplerIter {
public:
	XojPopplerIter(XojPopplerDocument doc, GooList *items);
	virtual ~XojPopplerIter();

public:
	bool next();
	bool isOpen();
	XojPopplerIter * getChildIter();
	XojPopplerAction * getAction();

private:
	static String unicodeToChar(Unicode * unicode, int len);

private:
	XojPopplerDocument doc;
	GooList *items;
	int index;
};

#endif /* __XOJPOPPLERITER_H__ */
