/*
 * Xournal++
 *
 * Displays a text Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

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

private:
    const Link* link;
};
