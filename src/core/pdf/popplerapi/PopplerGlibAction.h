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

#include <memory>  // for shared_ptr
#include <string>  // for string

#include <poppler.h>  // for PopplerAction, PopplerDocument

#include "pdf/base/XojPdfAction.h"  // for XojPdfAction
#include "util/raii/GObjectSPtr.h"  // for GObjectSPtr

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
    xoj::util::raii::GObjectSPtr<PopplerDocument> document;
    std::shared_ptr<const LinkDestination> destination;
    std::string title;
};
