#include "LinkView.h"

#include "model/Link.h"               // for Link
#include "util/Color.h"               // for cairo_set_source_rgbi
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard

using namespace xoj::view;

LinkView::LinkView(const Link* link): link(link) {}

void LinkView::draw(const Context& ctx) const {
    if (link->isInEditing()) {
        return;
    }

    xoj::util::CairoSaveGuard saveGuard(ctx.cr);

    cairo_set_operator(ctx.cr, CAIRO_OPERATOR_ATOP);
    Util::cairo_set_source_rgbi(ctx.cr, link->getColor());

    cairo_translate(ctx.cr, link->getX(), link->getY());

    cairo_show_text(ctx.cr, link->getText().c_str());
}