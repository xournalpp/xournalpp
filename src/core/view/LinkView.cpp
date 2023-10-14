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
    xoj::util::CairoSaveGuard saveGuard(ctx.cr);

    if (link->isHighlighted() || link->isSelected()) {
        Util::cairo_set_source_rgbi(ctx.cr, LINE_COLOR);
        cairo_rectangle(ctx.cr, link->getX(), link->getY(), link->getElementWidth(), link->getElementHeight());
        cairo_set_line_width(ctx.cr, LINE_WIDTH);
        cairo_stroke(ctx.cr);
    }


    cairo_set_operator(ctx.cr, CAIRO_OPERATOR_SOURCE);
    Util::cairo_set_source_rgbi(ctx.cr, link->getColor());

    cairo_tag_begin(ctx.cr, CAIRO_TAG_LINK, ("uri='" + link->getUrl() + "'").c_str());

    cairo_translate(ctx.cr, link->getX() + (Link::PADDING / 2), link->getY() + (Link::PADDING / 2));

    auto layout = initPango(ctx.cr, link);
    const std::string& content = link->getText();
    pango_layout_set_text(layout.get(), content.c_str(), static_cast<int>(content.length()));

    pango_cairo_show_layout(ctx.cr, layout.get());

    cairo_tag_end(ctx.cr, CAIRO_TAG_LINK);
}
