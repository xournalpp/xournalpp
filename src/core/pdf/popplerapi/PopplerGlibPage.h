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
    PopplerGlibPage(PopplerPage* page, PopplerDocument* doc);
    PopplerGlibPage(const PopplerGlibPage& other);
    virtual ~PopplerGlibPage();
    PopplerGlibPage& operator=(const PopplerGlibPage& other);

public:
    double getWidth() const override;
    double getHeight() const override;

    void render(cairo_t* cr, bool forPrinting = false) override;  // NOLINT(google-default-arguments)

    std::vector<XojPdfRectangle> findText(std::string& text) override;

    std::string selectText(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;

    cairo_region_t* selectTextRegion(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;

    TextSelection selectTextLines(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) override;

    auto getLinks() -> std::vector<Link> override;

    int getPageId() override;

private:
    PopplerPage* page;
    PopplerDocument* document;
};
