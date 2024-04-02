#include "TexImageView.h"

#include <cairo.h>             // for cairo_paint_with_alpha, cairo_scale
#include <glib.h>              // for g_warning
#include <poppler-document.h>  // for poppler_document_get_n_pages, poppler_...
#include <poppler-page.h>      // for poppler_page_render, poppler_page_get_...
#include <poppler.h>           // for PopplerPage, PopplerDocument, g_clear_...

#include "model/TexImage.h"           // for TexImage
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "util/raii/GObjectSPtr.h"    // for GObjectSPtr
#include "view/View.h"                // for Context, OPACITY_NO_AUDIO, view

using namespace xoj::view;

TexImageView::TexImageView(const TexImage* texImage): texImage(texImage) {}

TexImageView::~TexImageView() = default;

void TexImageView::draw(const Context& ctx) const {
    PopplerDocument* pdf = texImage->getPdf();
    if (pdf == nullptr || poppler_document_get_n_pages(pdf) == 0) {
        g_warning("Got latex PDF without pages!: %s", texImage->getText().c_str());
        return;
    }

    cairo_t* cr = ctx.cr;
    util::CairoSaveGuard saveGuard(cr);
    applyTransform(cr, texImage);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    util::GObjectSPtr<PopplerPage> page{poppler_document_get_page(pdf, 0), util::adopt};

    // Make TeX images translucent when highlighting audio strokes as they can not have audio
    if (ctx.fadeOutNonAudio) {
        /**
         * Switch to a temporary surface, render the page, then switch back.
         * This sets the current pattern to the temporary surface.
         */
        cairo_push_group(cr);
        poppler_page_render(page.get(), cr);
        cairo_pop_group_to_source(cr);

        // paint the temporary surface with opacity level
        cairo_paint_with_alpha(cr, OPACITY_NO_AUDIO);
    } else {
        poppler_page_render(page.get(), cr);
    }
}
