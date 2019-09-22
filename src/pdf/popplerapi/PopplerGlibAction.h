/*
 * Xournal++
 *
 * PDF Action Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/LinkDestination.h"
#include "pdf/base/XojPdfAction.h"

#include <XournalType.h>

#include <string>
using std::string;

#include <poppler.h>

class LinkDestination;


class PopplerGlibAction : public XojPdfAction
{
public:
	PopplerGlibAction(PopplerAction* action, PopplerDocument* document);
	virtual ~PopplerGlibAction();

public:
	virtual XojLinkDest* getDestination();
	virtual string getTitle();

private:
	void linkFromDest(LinkDestination* link, PopplerDest* pDest);

private:
	PopplerAction* action;
	PopplerDocument* document;
};

