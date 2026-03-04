#include "LinkPopover.h"

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...

#include "gui/PageView.h"     // for PageView
#include "gui/XournalView.h"  // for XournalView
#include "util/i18n.h"        // for _, FS
#include "util/safe_casts.h"  // for round_cast

LinkPopover::LinkPopover(XournalView* view): view(view) {
    this->popover = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_modal(this->popover, false);
    gtk_popover_set_constrain_to(this->popover, GTK_POPOVER_CONSTRAINT_WINDOW);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->label = GTK_LABEL(gtk_label_new(""));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(this->label), true, true, POPOVER_PADDING);
    gtk_container_add(GTK_CONTAINER(this->popover), vbox);
    gtk_widget_show_all(GTK_WIDGET(vbox));
}

LinkPopover::~LinkPopover() {
    if (this->link) {
        this->link->setSelected(false);
        this->link->setHighlighted(false);
        XojPageView* pageView = view->getViewFor(view->getCurrentPage());
        pageView->elementChanged(this->link);
    }
    if (this->popover) {
#if GTK_MAJOR_VERSION == 3
        gtk_widget_destroy(GTK_WIDGET(this->popover));
#else
        gtk_widget_unparent(GTK_WIDGET(this->popover));
#endif
    }
}

void LinkPopover::show() { gtk_widget_show(GTK_WIDGET(this->popover)); }

void LinkPopover::hide() { gtk_widget_hide(GTK_WIDGET(this->popover)); }

void LinkPopover::popup() { gtk_popover_popup(this->popover); }

void LinkPopover::popdown() { gtk_popover_popdown(this->popover); }

bool LinkPopover::hasLink() { return (this->link != nullptr); }

void LinkPopover::updateLabel(bool markup) {
    if (markup) {
        std::string url = this->link->getUrl();
        std::string explanation = FS(_F("Double click to edit. CTRL + click to open."));
        std::string str = "<a href=\"" + url + "\"> " + url + "</a> \n" + "<span size=\"smaller\"><i> " + explanation +
                          " </i></span>";
        gtk_label_set_markup(this->label, str.c_str());
    } else {
        gtk_label_set_text(this->label, this->link->getUrl().c_str());
    }
}

void LinkPopover::positionPopover() {
    if (this->link) {
        XojPageView* pageView = view->getViewFor(view->getCurrentPage());
        auto pos = pageView->getPixelPosition();
        auto zoom = pageView->getZoom();
        auto r = this->link->boundingRect();
        GdkRectangle rect{pos.x + round_cast<int>(r.x * zoom), pos.y + round_cast<int>(r.y * zoom),
                          round_cast<int>(r.width * zoom), round_cast<int>(r.height * zoom)};
        gtk_popover_set_pointing_to(this->popover, &rect);
    }
}

void LinkPopover::linkTo(Link* link) {
    this->link = link;
    positionPopover();
}
