/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "XojPopplerDocument.h"

#include "../../../model/LinkDestination.h"
#include <StringUtils.h>

#include "../poppler-0.24.1/poppler/Link.h"

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
