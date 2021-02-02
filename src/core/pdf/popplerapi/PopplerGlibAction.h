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

#include <memory>
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
    virtual std::shared_ptr<const LinkDestination> getDestination() override;
    virtual std::string getTitle() override;

private:
    virtual std::shared_ptr<const LinkDestination> getDestination(PopplerAction* action);
    void linkFromDest(LinkDestination& link, PopplerDest* pDest);

private:
    PopplerDocument* document;
    std::shared_ptr<const LinkDestination> destination;
    std::string title;
};
