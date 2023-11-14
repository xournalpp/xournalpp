#include "DocumentView.h"

#include <memory>  // for __shared_ptr_access, uni...
#include <vector>  // for vector

#include <glib.h>  // for g_message

#include "model/Layer.h"                     // for Layer
#include "model/XojPage.h"                   // for XojPage
#include "view/DebugShowRepaintBounds.h"     // for IF_DEBUG_REPAINT
#include "view/View.h"                       // for EditionTreatment, NORMAL...
#include "view/background/BackgroundView.h"  // for BackgroundFlags, Backgro...

#include "LayerView.h"  // for LayerView

/**
 * Mark stroke with Audio
 */
void DocumentView::setMarkAudioStroke(bool markAudioStroke) { this->markAudioStroke = markAudioStroke; }

void DocumentView::setPdfCache(PdfCache* cache) { pdfCache = cache; }

/**
 * Drawing first step
 * @param page The page to draw
 * @param cr Draw to this context
 * @param dontRenderEditingStroke false to draw currently drawing stroke
 */
void DocumentView::initDrawing(ConstPageRef page, cairo_t* cr, bool dontRenderEditingStroke) {
    this->cr = cr;
    this->page = page;
    this->dontRenderEditingStroke = dontRenderEditingStroke;
}

/**
 * Last step in drawing
 */
void DocumentView::finializeDrawing() {
    IF_DEBUG_REPAINT({
        double minX;
        double maxX;
        double minY;
        double maxY;
        cairo_clip_extents(cr, &minX, &minY, &maxX, &maxY);
        if (minX > 1 || minY > 1 || maxX < page->getWidth() - 1 || maxY < page->getHeight() - 1) {
            g_message("DBG:rerender area: %6.2f < x < %6.2f ; %6.2f < y < %6.2f", minX, maxX, minY, maxY);
            cairo_set_source_rgb(cr, 1, 0, 0);
            cairo_set_line_width(cr, 1);
            cairo_rectangle(cr, minX + 3, minY + 3, maxX - minX - 6, maxY - minY - 6);
            cairo_stroke(cr);
        } else {
            g_message("DBG:rerender complete");
        }
    });

    this->page = nullptr;
    this->cr = nullptr;
}

/**
 * Draw the background
 */
void DocumentView::drawBackground(xoj::view::BackgroundFlags bgFlags) const {
    auto bgView = xoj::view::BackgroundView::createForPage(page, bgFlags, pdfCache);
    bgView->draw(cr);
}

/**
 * Draw the full page, usually you would like to call this method
 * @param page The page to draw
 * @param cr Draw to thgis context
 * @param dontRenderEditingStroke false to draw currently drawing stroke
 * @param hideBackground true to hide the background
 */
void DocumentView::drawPage(ConstPageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hidePdfBackground,
                            bool hideImageBackground, bool hideRulingBackground) {
    initDrawing(page, cr, dontRenderEditingStroke);

    {  // Draw background
        xoj::view::BackgroundFlags bgFlags;
        bgFlags.showImage = (xoj::view::ImageBackgroundTreatment)!hideImageBackground;
        bgFlags.showPDF = (xoj::view::PDFBackgroundTreatment)!hidePdfBackground;
        bgFlags.showRuling = (xoj::view::RulingBackgroundTreatment)!hideRulingBackground;
        drawBackground(bgFlags);
    }

    xoj::view::Context context{cr, (xoj::view::NonAudioTreatment)this->markAudioStroke,
                               (xoj::view::EditionTreatment) !this->dontRenderEditingStroke, xoj::view::NORMAL_COLOR};
    for (const Layer* layer: page->getLayers()) {
        if (layer->isVisible()) {
            xoj::view::LayerView layerView(layer);
            layerView.draw(context);
        }
    }

    finializeDrawing();
}


void DocumentView::drawLayersOfPage(const LayerRangeVector& layerRange, ConstPageRef page, cairo_t* cr,
                                    bool dontRenderEditingStroke, bool hidePdfBackground, bool hideImageBackground,
                                    bool hideRulingBackground) {
    initDrawing(page, cr, dontRenderEditingStroke);

    {  // Draw background
        xoj::view::BackgroundFlags bgFlags;
        bgFlags.showImage = (xoj::view::ImageBackgroundTreatment)!hideImageBackground;
        bgFlags.showPDF = (xoj::view::PDFBackgroundTreatment)!hidePdfBackground;
        bgFlags.showRuling = (xoj::view::RulingBackgroundTreatment)!hideRulingBackground;
        drawBackground(bgFlags);
    }

    size_t layerCount = page->getLayerCount();
    std::vector<bool> visible(layerCount, false);

    for (LayerRangeEntry e: layerRange) {
        auto last = e.last;
        if (last >= layerCount) {
            last = layerCount - 1;
        }
        for (auto x = e.first; x <= last; x++) {
            visible[x] = true;
        }
    }

    xoj::view::Context context{cr, (xoj::view::NonAudioTreatment)this->markAudioStroke,
                               (xoj::view::EditionTreatment) !this->dontRenderEditingStroke, xoj::view::NORMAL_COLOR};
    auto visibilityIt = visible.begin();
    for (const Layer* l: page->getLayers()) {
        if (!*(visibilityIt++)) {
            continue;
        }
        xoj::view::LayerView layerView(l);
        layerView.draw(context);
    }

    finializeDrawing();
}
