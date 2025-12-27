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

#include <string>

#include <gtk/gtk.h>  // for GtkIMContext, GtkTextIter, GtkWidget

#include "model/Link.h"  // for Link

class XournalView;
class XojPageView;

class LinkPopover {
public:
    LinkPopover(XournalView* view);
    ~LinkPopover();
    void hide();
    void show();
    void show_all();
    void popup();
    void popdown();
    void updateLabel(bool markup);
    void positionPopover();
    bool hasLink();

    void linkTo(Link* link);
    inline GtkPopover* getPopover() const { return this->popover; }
    inline Link* getLink() const { return this->link; }

private:
    XournalView* view;

    GtkPopover* popover = nullptr;
    GtkLabel* label = nullptr;
    Link* link = nullptr;

    static constexpr int POPOVER_PADDING = 2;
};
