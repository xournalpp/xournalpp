#include "DocumentView.h"

#include <config-debug.h>
#include <config.h>

#include "background/MainBackgroundPainter.h"
#include "control/tools/EditSelection.h"
#include "control/tools/Selection.h"
#include "model/BackgroundImage.h"
#include "model/Layer.h"
#include "model/eraser/EraseableStroke.h"

#include "StrokeView.h"
#include "TextView.h"


DocumentView::DocumentView() { this->backgroundPainter = new MainBackgroundPainter(); }

DocumentView::~DocumentView() {
    delete this->backgroundPainter;
    this->backgroundPainter = nullptr;
}

/**
 * Mark stroke with Audio
 */
void DocumentView::setMarkAudioStroke(bool markAudioStroke) { this->markAudioStroke = markAudioStroke; }

void DocumentView::applyColor(cairo_t* cr, Stroke* s) {
    if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
        if (s->getFill() != -1) {
            applyColor(cr, s, s->getFill());
        } else {
            applyColor(cr, s, 120);
        }
    } else {
        applyColor(cr, static_cast<Element*>(s));
    }
}

void DocumentView::applyColor(cairo_t* cr, Element* e, int alpha) { applyColor(cr, e->getColor(), alpha); }

void DocumentView::applyColor(cairo_t* cr, int c, int alpha) {
    double r = ((c >> 16) & 0xff) / 255.0;
    double g = ((c >> 8) & 0xff) / 255.0;
    double b = (c & 0xff) / 255.0;

    cairo_set_source_rgba(cr, r, g, b, alpha / 255.0);
}

void DocumentView::drawStroke(cairo_t* cr, Stroke* s, int startPoint, double scaleFactor, bool changeSource,
                              bool noAlpha) const {
    if (s->getPointCount() < 2) {
        // Should not happen
        g_warning("DocumentView::drawStroke empty stroke...");
        return;
    }

    StrokeView sv(cr, s, startPoint, scaleFactor, noAlpha);

    if (changeSource) {
        sv.changeCairoSource(this->markAudioStroke);
    }

    sv.paint(this->dontRenderEditingStroke);
}

void DocumentView::drawText(cairo_t* cr, Text* t) {
    if (t->isInEditing()) {
        return;
    }

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    applyColor(cr, t);

    TextView::drawText(cr, t);
}

void DocumentView::drawImage(cairo_t* cr, Image* i) {
    cairo_matrix_t defaultMatrix = {0};
    cairo_get_matrix(cr, &defaultMatrix);

    cairo_surface_t* img = i->getImage();
    int width = cairo_image_surface_get_width(img);
    int height = cairo_image_surface_get_height(img);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    double xFactor = i->getElementWidth() / width;
    double yFactor = i->getElementHeight() / height;

    cairo_scale(cr, xFactor, yFactor);

    cairo_set_source_surface(cr, img, i->getX() / xFactor, i->getY() / yFactor);
    cairo_paint(cr);

    cairo_set_matrix(cr, &defaultMatrix);
}

void DocumentView::drawTexImage(cairo_t* cr, TexImage* texImage) {
    cairo_matrix_t defaultMatrix = {0};
    cairo_get_matrix(cr, &defaultMatrix);

    PopplerDocument* pdf = texImage->getPdf();
    cairo_surface_t* img = texImage->getImage();

    if (pdf != nullptr) {
        if (poppler_document_get_n_pages(pdf) < 1) {
            g_warning("Got latex PDf without pages!: %s", texImage->getText().c_str());
            return;
        }

        PopplerPage* page = poppler_document_get_page(pdf, 0);

        double pageWidth = 0;
        double pageHeight = 0;
        poppler_page_get_size(page, &pageWidth, &pageHeight);

        double xFactor = texImage->getElementWidth() / pageWidth;
        double yFactor = texImage->getElementHeight() / pageHeight;

        cairo_translate(cr, texImage->getX(), texImage->getY());
        cairo_scale(cr, xFactor, yFactor);
        poppler_page_render(page, cr);
    } else if (img != nullptr) {
        int width = cairo_image_surface_get_width(img);
        int height = cairo_image_surface_get_height(img);

        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

        double xFactor = texImage->getElementWidth() / width;
        double yFactor = texImage->getElementHeight() / height;

        cairo_scale(cr, xFactor, yFactor);

        cairo_set_source_surface(cr, img, texImage->getX() / xFactor, texImage->getY() / yFactor);
        cairo_paint(cr);
    }

    cairo_set_matrix(cr, &defaultMatrix);
}

void DocumentView::drawElement(cairo_t* cr, Element* e) const {
    if (e->getType() == ELEMENT_STROKE) {
        drawStroke(cr, dynamic_cast<Stroke*>(e));
    } else if (e->getType() == ELEMENT_TEXT) {
        drawText(cr, dynamic_cast<Text*>(e));
    } else if (e->getType() == ELEMENT_IMAGE) {
        drawImage(cr, dynamic_cast<Image*>(e));
    } else if (e->getType() == ELEMENT_TEXIMAGE) {
        drawTexImage(cr, dynamic_cast<TexImage*>(e));
    }
}

/**
 * Draw a single layer
 * @param cr Draw to thgis context
 * @param l The layer to draw
 */
void DocumentView::drawLayer(cairo_t* cr, Layer* l) {
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
    int drawn = 0;
    int notDrawn = 0;
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
    for (Element* e: *l->getElements()) {
#ifdef DEBUG_SHOW_ELEMENT_BOUNDS
        cairo_set_source_rgb(cr, 0, 1, 0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
        cairo_stroke(cr);
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
        // cairo_new_path(cr);

        if (this->lX != -1) {
            if (e->intersectsArea(this->lX, this->lY, this->width, this->height)) {
                drawElement(cr, e);
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
                drawn++;
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
            }
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
            else {
                notDrawn++;
            }
#endif  // DEBUG_SHOW_REPAINT_BOUNDS

        } else {
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
            drawn++;
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
            drawElement(cr, e);
        }
    }

#ifdef DEBUG_SHOW_REPAINT_BOUNDS
    g_message("DBG:DocumentView: draw %i / not draw %i", drawn, notDrawn);
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
}

void DocumentView::paintBackgroundImage() {
    GdkPixbuf* pixbuff = page->getBackgroundImage().getPixbuf();
    if (pixbuff) {
        cairo_matrix_t matrix = {0};
        cairo_get_matrix(cr, &matrix);

        int width = gdk_pixbuf_get_width(pixbuff);
        int height = gdk_pixbuf_get_height(pixbuff);

        double sx = page->getWidth() / width;
        double sy = page->getHeight() / height;

        cairo_scale(cr, sx, sy);

        gdk_cairo_set_source_pixbuf(cr, pixbuff, 0, 0);
        cairo_paint(cr);

        cairo_set_matrix(cr, &matrix);
    }
}

void DocumentView::drawSelection(cairo_t* cr, ElementContainer* container) {
    for (Element* e: *container->getElements()) {
        drawElement(cr, e);
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
void DocumentView::drawBackground() {
    PageType pt = page->getBackgroundType();
    if (pt.isPdfPage()) {
        // Handled in PdfView
    } else if (pt.isImagePage()) {
        paintBackgroundImage();
    } else {
        backgroundPainter->paint(pt, cr, page);
    }
}

/**
 * Draw background if there is no background shown, like in GIMP etc.
 */
void DocumentView::drawTransparentBackgroundPattern() {
    Util::cairo_set_source_rgbi(cr, 0x666666);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    Util::cairo_set_source_rgbi(cr, 0x999999);

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
void DocumentView::drawPage(PageRef page, cairo_t* cr, bool dontRenderEditingStroke, bool hideBackground) {
    initDrawing(page, cr, dontRenderEditingStroke);

    bool backgroundVisible = page->isLayerVisible(0);

    if (!hideBackground && backgroundVisible) {
        drawBackground();
    }

    if (!backgroundVisible) {
        drawTransparentBackgroundPattern();
    }

    int layer = 0;
    for (Layer* l: *page->getLayers()) {
        if (!page->isLayerVisible(l)) {
            continue;
        }

        drawLayer(cr, l);
        layer++;
    }

    finializeDrawing();
}
