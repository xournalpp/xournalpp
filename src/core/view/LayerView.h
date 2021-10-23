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
     * @brief Draws the portion of a Layer determined by drawArea
     */
    void draw(const Context& ctx, const xoj::util::Rectangle<double>& drawArea) const;

    /**
     * @brief Draws the entire Layer
     */
    void draw(const Context& ctx) const;

private:
    const Layer* layer;
};
