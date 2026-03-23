/*
 * Xournal++
 *
 * Link handler (for Link Tool)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <gdk/gdk.h>  // for GdkEventKey
#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

#include "control/zoom/ZoomListener.h"  // for ZoomListener
#include "model/OverlayBase.h"          // for OverlayBase
#include "model/PageRef.h"              // for PageRef

class Control;
class XournalView;
class XojPageView;
class LinkPopover;
class Link;

namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class OverlayView;
class Repaintable;
class LinkHighlightView;
};  // namespace xoj::view

class LinkHandler: public ZoomListener, public OverlayBase {
public:
    LinkHandler(XournalView* view);
    ~LinkHandler();

    /* Called on mouse double click event
     * -> Opens the link editor dialog, or tries to open the link when controlDown = CTRL */
    void startEditing(const PageRef& page, const int x, const int y);

    /* Called on mouse single click event
     * -> Make link permanently selected (red border + popover), or tries to open the link when contrlDown = CTRL */
    void select(const PageRef& page, const int x, const int y, const bool controlDown, XojPageView* pageView);

    /** Called on mouse move / hover event
     * -> Make link temporary highlighted (red border + popover) as long mouse above element */
    void highlight(const PageRef& page, const int x, const int y, XojPageView* pageView);

    void zoomChanged() override;

    auto createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView>;

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::LinkHighlightView>>& getViewPool() const {
        return viewPool;
    }

    std::optional<xoj::util::Rectangle<double>> getHighlightRect() const;
    std::optional<xoj::util::Rectangle<double>> getSelectRect() const;

private:
    XournalView* view;
    Control* control;

    auto isHighlighted(const Link* link) const -> bool;
    auto isSelected(const Link* link) const -> bool;

    // There is always at most one selected link and one highlighted link.
    // Both can be present at the same time, when one link being selected another one is highlighted.
    std::unique_ptr<LinkPopover> highlightPopover;
    std::unique_ptr<LinkPopover> selectPopover;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::LinkHighlightView>> viewPool;
};
