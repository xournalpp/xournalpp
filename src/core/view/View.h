/*
 * Xournal++
 *
 * Namespace for view related classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>

namespace xoj {
namespace view {

class ElementView {
public:
    virtual ~ElementView() = default;
    virtual void draw(cairo_t* cr) const = 0;
};
};  // namespace view
};  // namespace xoj
