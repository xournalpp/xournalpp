/*
 * Xournal++
 *
 * Displays a Layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Rectangle.h"

#include "View.h"

class Layer;

class xoj::view::LayerView {
public:
    LayerView(const Layer* layer);

    /**
     * @brief Draws the entire Layer
     */
    void draw(const Context& ctx) const;

    const Layer* getLayer() const;

private:
    const Layer* layer;
};
