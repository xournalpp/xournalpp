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
    virtual double getWidth();
    virtual double getHeight();

    virtual void render(cairo_t* cr, bool forPrinting = false);  // NOLINT(google-default-arguments)

    virtual std::vector<XojPdfRectangle> findText(std::string& text);

    std::string selectText(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;

    cairo_region_t* selectTextRegion(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;

    TextSelection selectTextLines(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;


    virtual int getPageId();

private:
    PopplerPage* page;
};
