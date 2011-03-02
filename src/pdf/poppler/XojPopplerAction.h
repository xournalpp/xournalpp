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

#include "../../model/LinkDestination.h"
#include "../../util/String.h"

#include <poppler/Link.h>

class XojPopplerAction  {
public:
	XojPopplerAction(XojPopplerDocument doc, LinkAction * linkAction, String title);
	virtual ~XojPopplerAction();

public:
	XojLinkDest * getDestination();
	String getTitle();

private:
	void linkFromDest(LinkDestination *link, LinkDest *dest);

private:
	XojPopplerDocument doc;
	LinkAction * linkAction;
	String title;
};

#endif /* __XOJPOPPLERACTION_H__ */
