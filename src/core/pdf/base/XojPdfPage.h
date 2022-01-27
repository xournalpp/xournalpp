/*
 * Xournal++
 *
 * PDF Page Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // std::shared_ptr
#include <string>
#include <vector>

#include <cairo/cairo.h>

enum XojPdfPageSelectionStyle { XOJ_PDF_SELECTION_GLYPH, XOJ_PDF_SELECTION_WORD, XOJ_PDF_SELECTION_LINE };

class XojPdfRectangle {
public:
    XojPdfRectangle();
    XojPdfRectangle(double x1, double y1, double x2, double y2);

public:
    double x1 = -1;
    double y1 = -1;
    double x2 = -1;
    double y2 = -1;
};

class XojPdfPage {
public:
    XojPdfPage();
    virtual ~XojPdfPage();

public:
    virtual double getWidth() = 0;
    virtual double getHeight() = 0;

    virtual void render(cairo_t* cr, bool forPrinting = false) = 0;

    virtual std::vector<XojPdfRectangle> findText(std::string& text) = 0;

    virtual std::string selectHeadTailText(const XojPdfRectangle& rect,
                                           XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH) = 0;
    virtual cairo_region_t* selectHeadTailTextRegion(const XojPdfRectangle& rect,
                                                     XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH) = 0;
    virtual void selectHeadTailFinally(const XojPdfRectangle& se, cairo_region_t** region,
                                       std::vector<XojPdfRectangle>* rects, std::string* text,
                                       XojPdfPageSelectionStyle style = XOJ_PDF_SELECTION_GLYPH) = 0;
    virtual void selectAreaFinally(const XojPdfRectangle& rect, cairo_region_t** region,
                                   std::vector<XojPdfRectangle>* rects, std::string* text) = 0;

    virtual int getPageId() = 0;

private:
};

typedef std::shared_ptr<XojPdfPage> XojPdfPageSPtr;
