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

#include "model/LinkDestination.h"
#include "pdf/popplerdirect/poppler-0.24.1/poppler/Link.h"
#include "XojPopplerDocument.h"

#include <StringUtils.h>

class XojPopplerAction
{
public:
	XojPopplerAction(XojPopplerDocument doc, LinkAction* linkAction, string title);
	virtual ~XojPopplerAction();

public:
	XojLinkDest* getDestination();
	string getTitle();

private:
	void linkFromDest(LinkDestination* link, LinkDest* dest);

private:
	XOJ_TYPE_ATTRIB;

	XojPopplerDocument doc;
	LinkAction* linkAction;
	string title;
};
