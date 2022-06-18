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

#include <string>  // for string

#include <poppler.h>  // for PopplerAction, PopplerDocument

#include "model/LinkDestination.h"  // for LinkDestination (ptr only), XojLi...
#include "pdf/base/XojPdfAction.h"  // for XojPdfAction


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
