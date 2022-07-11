/*
 * Xournal++
 *
 * Paints a page to a cairo context
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "model/PageRef.h"  // for PageRef
#include "util/ElementRange.h"

class PdfCache;

namespace xoj::view {
struct BackgroundFlags;
};

class DocumentView {
public:
    DocumentView() = default;
    virtual ~DocumentView() = default;

public:
    /**
     * Draw the full page, usually you would like to call this method
     * @param page The page to draw
     * @param cr Draw to thgis context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     * @param hidePdfBackground true to hide the PDF background
     * @param hideImageBackground true to hide the PDF background
     * @param hideRulingBacground true to hide the ruling background
     */
    void drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hidePdfBackground = false,
                  bool hideImageBackground = false, bool hideRulingBackground = false);

    /**
     * Only draws the prescribed layers of the given page, regardless of the layer's current visibility.
     * @param layerRange Range of layers to draw
     * @param page The page to draw
     * @param cr Draw to this context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     * @param hidePdfBackground true to hide the PDF background
     * @param hideImageBackground true to hide the PDF background
     * @param hideRulingBacground true to hide the ruling background
     */
    void drawLayersOfPage(const LayerRangeVector& layerRange, PageRef page, cairo_t* cr, bool dontRenderEditingStroke,
                          bool hidePdfBackground = false, bool hideImageBackground = false,
                          bool hideRulingBackground = false);

    /**
     * Mark stroke with Audio
     */
    void setMarkAudioStroke(bool markAudioStroke);

    // API for special drawing, usually you won't call this methods
public:
    void setPdfCache(PdfCache* cache);

    /**
     * Drawing first step
     * @param page The page to draw
     * @param cr Draw to thgis context
     * @param dontRenderEditingStroke false to draw currently drawing stroke
     */
    void initDrawing(PageRef page, cairo_t* cr, bool dontRenderEditingStroke);

    /**
     * Draw the background
     */
    void drawBackground(xoj::view::BackgroundFlags bgFlags) const;

    /**
     * Draw background if there is no background shown, like in GIMP etc.
     */
    void drawTransparentBackgroundPattern();

    /**
     * Last step in drawing
     */
    void finializeDrawing();

private:
    cairo_t* cr = nullptr;
    PageRef page = nullptr;
    PdfCache* pdfCache = nullptr;
    bool dontRenderEditingStroke = false;
    bool markAudioStroke = false;

};
