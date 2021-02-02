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

#include "model/LinkDestination.h"


class XojPdfAction {
public:
    XojPdfAction();
    virtual ~XojPdfAction();

public:
    virtual std::shared_ptr<const LinkDestination> getDestination() = 0;
    virtual std::string getTitle() = 0;

private:
};
