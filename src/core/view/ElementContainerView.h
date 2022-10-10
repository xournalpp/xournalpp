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

class ElementContainer;

namespace xoj::view {
class Context;

class ElementContainerView {
public:
    ElementContainerView(const ElementContainer* container);

    /**
     * @brief Draws the container to the given context
     */
    void draw(const Context& ctx) const;

private:
    const ElementContainer* container;
};
};  // namespace xoj::view
