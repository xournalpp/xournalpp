#include "DocumentView.h"

#include "control/tools/EditSelection.h"
#include "control/tools/Selection.h"
#include "model/Layer.h"
#include "model/eraser/ErasableStroke.h"
#include "view/background/MainBackgroundPainter.h"

#include "ImageBackgroundView.h"
#include "LayerView.h"
#include "StrokeView.h"

using xoj::util::Rectangle;

DocumentView::DocumentView() { this->backgroundPainter = new MainBackgroundPainter(); }

DocumentView::~DocumentView() {
    delete this->backgroundPainter;
    this->backgroundPainter = nullptr;
}

/**
 * Mark stroke with Audio
 */
void DocumentView::setMarkAudioStroke(bool markAudioStroke) { this->markAudioStroke = markAudioStroke; }

void DocumentView::drawSelection(cairo_t* cr, ElementContainer* container) {
    xoj::view::Context context{cr, (xoj::view::NonAudioTreatment)this->markAudioStroke,
                               (xoj::view::EditionTreatment)this->dontRenderEditingStroke, xoj::view::NORMAL_COLOR};
    for (Element* e: *container->getElements()) {
        auto elementView = xoj::view::ElementView::createFromElement(e);
        elementView->draw(context);
    }
}

void DocumentView::limitArea(double x, double y, double width, double height) {
    this->lX = x;
    this->lY = y;
    this->lWidth = width;
    this->lHeight = height;
}

/**
 * Drawing first step
 * @param page The page to draw
 * @param cr Draw to thgis context
 * @param dontRenderEditingStroke false to draw currently drawing stroke
 */
void DocumentView::initDrawing(PageRef page, cairo_t* cr, bool dontRenderEditingStroke) {
    this->cr = cr;
    this->page = page;
    this->width = page->getWidth();
    this->height = page->getHeight();
    this->dontRenderEditingStroke = dontRenderEditingStroke;
}

/**
 * Last step in drawing
 */
void DocumentView::finializeDrawing() {
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
    if (this->lX != -1) {
        g_message("DBG:repaint area");
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, this->lX + 3, this->lY + 3, this->lWidth - 6, this->lHeight - 6);
        cairo_stroke(cr);
    } else {
        g_message("DBG:repaint complete");
    }
#endif  // DEBUG_SHOW_REPAINT_BOUNDS

    this->lX = -1;
    this->lY = -1;
    this->lWidth = -1;
    this->lHeight = -1;

    this->page = nullptr;
    this->cr = nullptr;
}

/**
 * Draw the background
 */
void DocumentView::drawBackground(bool hidePdfBackground, bool hideImageBackground, bool hideRulingBackground) {
    PageType pt = page->getBackgroundType();
    if (pt.isPdfPage()) {
        // Handled in PdfView
    } else if (pt.isImagePage() && !hideImageBackground) {
        xoj::view::ImageBackgroundView bgView(page->getBackgroundImage(), page->getWidth(), page->getHeight());
        bgView.draw(cr);
    } else if (!hideRulingBackground) {
        backgroundPainter->paint(pt, cr, page);
    }
}

/**
 * Draw background if there is no background shown, like in GIMP etc.
 */
void DocumentView::drawTransparentBackgroundPattern() {
    Util::cairo_set_source_rgbi(cr, Color(0x666666U));
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, Color(0x999999U));

    bool second = false;
    for (int y = 0; y < height; y += 8) {
        second = !second;
        for (int x = second ? 8 : 0; x < width; x += 16) {
            cairo_rectangle(cr, x, y, 8, 8);
            cairo_fill(cr);
        }
    }
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

    bool backgroundVisible = page->isLayerVisible(0);

    if (backgroundVisible) {
        drawBackground(hidePdfBackground, hideImageBackground, hideRulingBackground);
    }

    if (!backgroundVisible) {
        drawTransparentBackgroundPattern();
    }

    xoj::view::Context context{cr, (xoj::view::NonAudioTreatment)this->markAudioStroke,
                               (xoj::view::EditionTreatment)this->dontRenderEditingStroke, xoj::view::NORMAL_COLOR};
    const Rectangle<double> drawArea{this->lX, this->lY, this->lWidth, this->lHeight};
    for (Layer* layer: *page->getLayers()) {
        if (page->isLayerVisible(layer)) {
            xoj::view::LayerView layerView(layer);
            if (this->lX == -1) {
                layerView.draw(context);
            } else {
                layerView.draw(context, drawArea);
            }
        }
    }

    finializeDrawing();
}
