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

#ifndef __XOJPOPPLERACTION_H__
#define __XOJPOPPLERACTION_H__

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

#endif /* __XOJPOPPLERACTION_H__ */
