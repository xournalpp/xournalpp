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

#include <cstdint>  // for uint8_t
#include <memory>   // for shared_ptr
#include <string>   // for string
#include <vector>   // for vector

#include <cairo.h>  // for cairo_region_t, cairo_t
#include <glib.h>   // for GURI

#include "util/raii/CairoWrappers.h"

#include "XojPdfAction.h"

class XojPdfLink;

/// Determines how text is selected on a user action.
enum class XojPdfPageSelectionStyle : uint8_t {
    /// Standard selection, where all text between start and end positions is selected.
    Linear,
    /// Select a single word.
    Word,
    /// Select a single line.
    Line,
    /// Select an area.
    Area,
};

class XojPdfRectangle {
public:
    XojPdfRectangle() = default;
    XojPdfRectangle(double x1, double y1, double x2, double y2);

public:
    double x1 = -1;
    double y1 = -1;
    double x2 = -1;
    double y2 = -1;
};

class XojPdfPage {
public:
    struct TextSelection {
        xoj::util::CairoRegionSPtr region;
        std::vector<XojPdfRectangle> rects;
    };

    struct Link {
        XojPdfRectangle bounds;
        std::unique_ptr<XojPdfAction> action;
    };

    virtual double getWidth() const = 0;
    virtual double getHeight() const = 0;

    /**
     * Renders the page to the given cairo context.
     * Use render() for rendering to a screen (or a raster image format),
     * Use renderForPrinting() for exports to vector formats (not sure it will be vectorial, but the quality is better)
     *
     * See https://poppler.freedesktop.org/api/glib/poppler-Poppler-Page.html#poppler-page-render-for-printing
     * for actual differences between the two functions in the poppler based implementation.
     */
    virtual void render(cairo_t* cr) const = 0;
    virtual void renderForPrinting(cairo_t* cr) const = 0;

    virtual std::vector<XojPdfRectangle> findText(const std::string& text) = 0;

    /// Retrieve the text contained in the provided rectangle using the given
    /// selection style.
    /// @param rect start and end points
    /// @param style The text selection style
    /// @return The selected text.
    virtual std::string selectText(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) = 0;

    /// Retrieve the cairo_region_t that encompasses the text that would be
    /// selected in the given rectangle with the given text selection style.
    /// @param rect start and end points
    /// @param style The text selection style
    /// @return A region that contains the text that would be selected.
    virtual cairo_region_t* selectTextRegion(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) = 0;

    /// Retrieve the set of rectangles that represent each line of text selected
    /// in the given rectangle with the given text selection style.
    /// @param rect start and end points
    /// @param style The text selection style
    /// @return The rectangles that cover the text that would be selected.
    virtual TextSelection selectTextLines(const XojPdfRectangle& rect, XojPdfPageSelectionStyle style) = 0;

    /**
     * @return A list of Links in the current page.
     */
    virtual auto getLinks() -> std::vector<Link> = 0;

    virtual int getPageId() const = 0;

private:
};

typedef std::shared_ptr<XojPdfPage> XojPdfPageSPtr;
