/*
 * Xournal++
 *
 * Displays a link element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <pango/pangocairo.h>  // for PangoLayout, cairo_t

#include "util/Color.h"  // for Color
#include "util/raii/GObjectSPtr.h"

#include "View.h"  // for ElementView

class Link;

class xoj::view::LinkView: public xoj::view::ElementView {
public:
    LinkView(const Link* link);
    ~LinkView() = default;

    /**
     * Draws a Link model to a cairo surface
     */
    void draw(const Context& ctx) const override;

    /**
     * Initialize a Pango layout
     */
    static xoj::util::GObjectSPtr<PangoLayout> initPango(cairo_t* cr, const Link* l);

private:
    const Link* link;

    static constexpr double LINE_WIDTH = 1.0;
    static constexpr Color LINE_COLOR{255, 0, 0};
};
