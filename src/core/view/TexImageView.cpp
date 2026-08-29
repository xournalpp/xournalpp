#include "TexImageView.h"

#include <string>  // for string

#include <cairo.h>    // for cairo_paint_with_alpha, cairo_scale
#include <glib.h>     // for g_warning
#include <poppler.h>  // for PopplerPage, PopplerDocument, g_clear_...

#include "model/TexImage.h"  // for TexImage
#include "util/Matrix.h"     // for Matrix
#include "util/raii/CairoWrappers.h"
#include "view/View.h"  // for Context, OPACITY_NO_AUDIO, view

using namespace xoj::view;

TexImageView::TexImageView(const TexImage* texImage): texImage(texImage) {}

TexImageView::~TexImageView() = default;

void TexImageView::draw(const Context& ctx) const {
    cairo_t* cr = ctx.cr;
    xoj::util::CairoSaveGuard save(cr);

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    texImage->getTransformation().transformCairo(cr);

    if (PopplerDocument* pdf = texImage->getPdf(); pdf != nullptr) {
        if (poppler_document_get_n_pages(pdf) < 1) {
            g_warning("Got latex PDF without pages!: %s", texImage->getText().c_str());
            return;
        }

        PopplerPage* page = poppler_document_get_page(pdf, 0);

        auto surfType = cairo_surface_get_type(cairo_get_target(cr));
        auto pageRenderFunction =
                surfType == CAIRO_SURFACE_TYPE_PDF || surfType == CAIRO_SURFACE_TYPE_PS ||
                                surfType == CAIRO_SURFACE_TYPE_SVG || surfType == CAIRO_SURFACE_TYPE_SCRIPT ||
                                surfType == CAIRO_SURFACE_TYPE_WIN32_PRINTING || surfType == CAIRO_SURFACE_TYPE_XML ||
                                surfType == CAIRO_SURFACE_TYPE_RECORDING ?
                        poppler_page_render_for_printing :
                        poppler_page_render;

        // Make TeX images translucent when highlighting audio strokes as they can not have audio
        if (ctx.fadeOutNonAudio) {
            /**
             * Switch to a temporary surface, render the page, then switch back.
             * This sets the current pattern to the temporary surface.
             */
            cairo_push_group(cr);
            pageRenderFunction(page, cr);
            cairo_pop_group_to_source(cr);

            // paint the temporary surface with opacity level
            cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
        } else {
            pageRenderFunction(page, cr);
        }

        g_clear_object(&page);
    } else if (cairo_surface_t* img = texImage->getImage(); img != nullptr) {
        cairo_set_source_surface(cr, img, 0, 0);

        // Make TeX images translucent when highlighting audio strokes as they can not have audio
        if (ctx.fadeOutNonAudio) {
            cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
        } else {
            cairo_paint(cr);
        }
    }
}
