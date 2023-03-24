#include "LinkView.h"

#include "model/Link.h"               // for Link
#include "util/Color.h"               // for cairo_set_source_rgbi
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard

using namespace xoj::view;

LinkView::LinkView(const Link* link): link(link) {}

auto LinkView::initPango(cairo_t* cr, const Link* l) -> xoj::util::GObjectSPtr<PangoLayout> {
    auto layout = l->createPangoLayout();
    pango_cairo_update_layout(cr, layout.get());
    pango_context_set_matrix(pango_layout_get_context(layout.get()), nullptr);

    return layout;
}

void LinkView::draw(const Context& ctx) const {
    if (link->isInEditing()) {
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(ctx.cr);

    cairo_set_operator(ctx.cr, CAIRO_OPERATOR_SOURCE);
    Util::cairo_set_source_rgbi(ctx.cr, link->getColor());

    cairo_translate(ctx.cr, link->getX(), link->getY());

    auto layout = initPango(ctx.cr, link);
    const std::string& content = link->getText();
    pango_layout_set_text(layout.get(), content.c_str(), static_cast<int>(content.length()));

    pango_cairo_show_layout(ctx.cr, layout.get());
}