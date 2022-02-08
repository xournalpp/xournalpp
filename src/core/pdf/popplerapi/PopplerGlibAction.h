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

#include <string>
#include <vector>

#include <poppler.h>

#include "model/LinkDestination.h"
#include "pdf/base/XojPdfAction.h"


class LinkDestination;


class PopplerGlibAction: public XojPdfAction {
public:
    PopplerGlibAction(PopplerAction* action, PopplerDocument* document);
    ~PopplerGlibAction() override;

public:
    XojLinkDest* getDestination() override;
    std::string getTitle() override;

private:
    void linkFromDest(LinkDestination* link, PopplerDest* pDest);

private:
    PopplerAction* action;
    PopplerDocument* document;
};
