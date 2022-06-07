#include "DocumentView.h"

#include "control/PdfCache.h"  // TODO: remove to cleanup MVC scheme
#include "model/BackgroundConfig.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/eraser/ErasableStroke.h"
#include "view/DebugShowRepaintBounds.h"
#include "view/background/BackgroundView.h"

#include "LayerView.h"
#include "StrokeView.h"

using xoj::util::Rectangle;

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
void DocumentView::initDrawing(PageRef page, cairo_t* cr, bool dontRenderEditingStroke) {
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
void DocumentView::drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hidePdfBackground,
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
    for (Layer* layer: *page->getLayers()) {
        if (layer->isVisible()) {
            xoj::view::LayerView layerView(layer);
            layerView.draw(context);
        }
    }

    finializeDrawing();
}
