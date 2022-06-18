/*
 * Xournal++
 *
 * Handles text search on a PDF page and in Xournal Texts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

#include <cairo.h>    // for cairo_t
#include <gdk/gdk.h>  // for GdkRGBA

#include "model/PageRef.h"        // for PageRef
#include "pdf/base/XojPdfPage.h"  // for XojPdfPageSPtr, XojPdfRectangle

class SearchControl {
public:
    SearchControl(const PageRef& page, XojPdfPageSPtr pdf);
    virtual ~SearchControl();

    bool search(std::string text, int* occures, double* top);
    void paint(cairo_t* cr, double zoom, const GdkRGBA& color);

private:
    void freeSearchResults();

private:
    PageRef page;
    XojPdfPageSPtr pdf;

    std::vector<XojPdfRectangle> results;
};
