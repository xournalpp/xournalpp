/*
 * Xournal++
 *
 * Displays the content of a selection
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "View.h"

class ElementContainer;

class xoj::view::SelectionView {
public:
    SelectionView(const ElementContainer* container);

    /**
     * @brief Draws the container to the given context
     */
    void draw(const Context& ctx) const;

private:
    const ElementContainer* container;
};
