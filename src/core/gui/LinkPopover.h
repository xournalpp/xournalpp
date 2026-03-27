/*
 * Xournal++
 *
 * Link popover gui (displayed above a selected or highlighted link)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>
#include <string>

#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

#include "model/Link.h"  // for Link

class XournalView;
class XojPageView;

class LinkPopover {
public:
    LinkPopover(XournalView* view, bool markup);
    ~LinkPopover();
    void hide();
    void show();
    void popup();
    void popdown();
    bool hasLink() const;
    std::optional<xoj::util::Rectangle<double>> getRect();

    void linkTo(Link* link);
    inline Link* getLink() const { return this->link; }

private:
    void positionPopover();
    void updateRect();
    void updateLabel();

private:
    XournalView* view;
    bool markup;

    GtkPopover* popover;
    GtkLabel* label = nullptr;
    Link* link = nullptr;
    std::optional<xoj::util::Rectangle<double>> rect = std::nullopt;

    static constexpr int POPOVER_PADDING = 2;
};
