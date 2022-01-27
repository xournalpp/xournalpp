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

    /**
     * get the selected text(from start point to end point) using poppler api
     * @param rect start and end points
     * @return the text we select
     */
    virtual std::string selectHeadTailText(const XojPdfRectangle& rect,
                                           XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH);

    /**
     * get the selected region(from start point to end point) using poppler api. only use for show selection
     * @param rect start and end points
     * @return the text we select
     */
    virtual cairo_region_t* selectHeadTailTextRegion(const XojPdfRectangle& rect,
                                                     XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH);

    /**
     * get the selection(selected region, text and lines from start point to end point) finally, ready for creating
     * strokes or do other operations
     * @param rect   (in)  start and end points
     * @param region (out) the selected region for showing selection
     * @param rects  (out) lines
     * @param text   (out) the text we select
     */
    virtual void selectHeadTailFinally(const XojPdfRectangle& rect, cairo_region_t** region,
                                       std::vector<XojPdfRectangle>* rects, std::string* text,
                                       XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH);
    /**
     * get the selection(selected region, text and lines only in the rectangle made by start and end points) finally,
     * ready for creating strokes or do other operations
     * @param rect rect start and end points
     * @param rect   (in)  start and end points
     * @param region (out) the selected region for showing selection
     * @param rects  (out) lines
     * @param text   (out) the text we select
     */
    virtual void selectAreaFinally(const XojPdfRectangle& rect, cairo_region_t** region,
                                   std::vector<XojPdfRectangle>* rects, std::string* text);

    virtual int getPageId();

private:
    PopplerPage* page;
    void amendHeadAndTail(std::vector<XojPdfRectangle>* rects, const cairo_region_t* region,
                          XojPdfPageSelectionStyle style);
};
