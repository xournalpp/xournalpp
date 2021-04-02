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

#include <cairo.h>

#include "Rectangle.h"

class XojPdfRectangle {
public:
    XojPdfRectangle();
    XojPdfRectangle(const Rectangle<double>& rect);
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

    virtual void render(cairo_t* cr, bool forPrinting = false) = 0; // NOLINT(google-default-arguments)

    /**
     * @param cr is the target context.
     * @param region is the region of this page to be rendered.
     *
     * Assumes forPrinting = false.
     */
    virtual void render(cairo_t* cr, XojPdfRectangle region);

    virtual std::vector<XojPdfRectangle> findText(std::string& text) = 0;

    virtual int getPageId() const = 0;

private:
};

typedef std::shared_ptr<XojPdfPage> XojPdfPageSPtr;
