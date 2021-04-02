/*
 * Xournal++
 *
 * PDF Page GLib Implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <poppler.h>

#include "pdf/base/XojPdfPage.h"


class PopplerGlibPage: public XojPdfPage {
public:
    PopplerGlibPage(PopplerPage* page);
    PopplerGlibPage(const PopplerGlibPage& other);
    virtual ~PopplerGlibPage();
    PopplerGlibPage& operator=(const PopplerGlibPage& other);

public:
    virtual double getWidth() override;
    virtual double getHeight() override;

    virtual void render(cairo_t* cr, bool forPrinting) override;

    virtual std::vector<XojPdfRectangle> findText(std::string& text) override;

    virtual int getPageId() const override;

private:
    PopplerPage* page;
};
