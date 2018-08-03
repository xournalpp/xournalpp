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
#include <poppler/Link.h>
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
	void linkFromDest(LinkDestination* link, const LinkDest* dest);

private:
	XOJ_TYPE_ATTRIB;

	XojPopplerDocument doc;
	LinkAction* linkAction;
	string title;
};
