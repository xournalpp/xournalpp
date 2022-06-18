#include "TexImageView.h"

#include <string>  // for string

#include <cairo.h>             // for cairo_paint_with_alpha, cairo_scale
#include <glib.h>              // for g_warning
#include <poppler-document.h>  // for poppler_document_get_n_pages, poppler_...
#include <poppler-page.h>      // for poppler_page_render, poppler_page_get_...
#include <poppler.h>           // for PopplerPage, PopplerDocument, g_clear_...

#include "model/TexImage.h"  // for TexImage
#include "view/View.h"       // for Context, OPACITY_NO_AUDIO, view

using namespace xoj::view;

TexImageView::TexImageView(const TexImage* texImage): texImage(texImage) {}

TexImageView::~TexImageView() = default;

void TexImageView::draw(const Context& ctx) const {

    cairo_t* cr = ctx.cr;
    cairo_save(cr);

    PopplerDocument* pdf = texImage->getPdf();
    cairo_surface_t* img = texImage->getImage();

    if (pdf != nullptr) {
        if (poppler_document_get_n_pages(pdf) < 1) {
            g_warning("Got latex PDF without pages!: %s", texImage->getText().c_str());
            return;
        }

        PopplerPage* page = poppler_document_get_page(pdf, 0);

        double pageWidth = 0;
        double pageHeight = 0;
        poppler_page_get_size(page, &pageWidth, &pageHeight);

        double xFactor = texImage->getElementWidth() / pageWidth;
        double yFactor = texImage->getElementHeight() / pageHeight;

        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_translate(cr, texImage->getX(), texImage->getY());
        cairo_scale(cr, xFactor, yFactor);

        // Make TeX images translucent when highlighting audio strokes as they can not have audio
        if (ctx.fadeOutNonAudio) {
            /**
             * Switch to a temporary surface, render the page, then switch back.
             * This sets the current pattern to the temporary surface.
             */
            cairo_push_group(cr);
            poppler_page_render(page, cr);
            cairo_pop_group_to_source(cr);

            // paint the temporary surface with opacity level
            cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
        } else {
            poppler_page_render(page, cr);
        }

        g_clear_object(&page);
    } else if (img != nullptr) {
        int width = cairo_image_surface_get_width(img);
        int height = cairo_image_surface_get_height(img);

        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

        double xFactor = texImage->getElementWidth() / width;
        double yFactor = texImage->getElementHeight() / height;

        cairo_scale(cr, xFactor, yFactor);

        cairo_set_source_surface(cr, img, texImage->getX() / xFactor, texImage->getY() / yFactor);
        // Make TeX images translucent when highlighting audio strokes as they can not have audio
        if (ctx.fadeOutNonAudio) {
            cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
        } else {
            cairo_paint(cr);
        }
    }

    cairo_restore(cr);
}
