#include "LinkPopover.h"

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...

#include "gui/PageView.h"     // for PageView
#include "gui/XournalView.h"  // for XournalView

LinkPopover::LinkPopover(XournalView* view): view(view) {
    this->popover = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_modal(popover, false);
    gtk_popover_set_constrain_to(this->popover, GTK_POPOVER_CONSTRAINT_WINDOW);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->label = GTK_LABEL(gtk_label_new(""));
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(this->label), true, true, POPOVER_PADDING);
    gtk_container_add(GTK_CONTAINER(this->popover), vbox);
}

LinkPopover::~LinkPopover() {
    gtk_widget_destroy(GTK_WIDGET(this->label));
    gtk_widget_destroy(GTK_WIDGET(this->popover));
}

void LinkPopover::show() { gtk_widget_show(GTK_WIDGET(this->popover)); }

void LinkPopover::show_all() { gtk_widget_show_all(GTK_WIDGET(this->popover)); }

void LinkPopover::hide() { gtk_widget_hide(GTK_WIDGET(this->popover)); }

void LinkPopover::popup() { gtk_popover_popup(this->popover); }

void LinkPopover::popdown() { gtk_popover_popdown(this->popover); }

bool LinkPopover::hasLink() { return (this->link != nullptr); }

void LinkPopover::updateLabel(bool markup) {
    if (markup) {
        std::string url = this->link->getUrl();
        std::string str = "<a href=\"" + url + "\"> " + url + "</a> \n" +
                          "<span size=\"smaller\"><i> Double click to edit. CTRL + click to open. </i></span>";
        gtk_label_set_markup(this->label, str.c_str());
    } else {
        gtk_label_set_text(this->label, this->link->getUrl().c_str());
    }
}

void LinkPopover::positionPopover() {
    if (this->link) {
        XojPageView* pageView = view->getViewFor(view->getCurrentPage());
        auto pos = pageView->getPixelPosition();
        GdkRectangle rect{pos.x + int(this->link->getX() * pageView->getZoom()),
                          pos.y + int(this->link->getY() * pageView->getZoom()),
                          int(this->link->getElementWidth() * pageView->getZoom()),
                          int(this->link->getElementHeight() * pageView->getZoom())};
        gtk_popover_set_pointing_to(this->popover, &rect);
    }
}

void LinkPopover::linkTo(Link* link) {
    this->link = link;
    positionPopover();
}
