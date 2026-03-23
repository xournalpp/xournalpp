#include "LinkPopover.h"

#include <gdk/gdk.h>  // for GdkRectangle, Gdk...

#include "gui/PageView.h"               // for PageView
#include "gui/XournalView.h"            // for XournalView
#include "gui/scroll/ScrollHandling.h"  // for getPosition
#include "util/XojMsgBox.h"             // for XojMsgBox
#include "util/gtk4_helper.h"           // for gtk_box_append...
#include "util/i18n.h"                  // for _, FS
#include "util/safe_casts.h"            // for round_cast

LinkPopover::LinkPopover(XournalView* view, bool markup): view(view), markup(markup) {
    this->popover = GTK_POPOVER(gtk_popover_new(view->getWidget()));
    gtk_popover_set_autohide(this->popover, false);
    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    this->label = GTK_LABEL(gtk_label_new(""));

    gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(this->label));
    gtk_box_set_spacing(GTK_BOX(vbox), POPOVER_PADDING);
    gtk_widget_set_hexpand(GTK_WIDGET(this->label), true);
    gtk_widget_set_vexpand(GTK_WIDGET(this->label), true);
    gtk_popover_set_child(GTK_POPOVER(this->popover), vbox);
#if GTK_MAJOR_VERSION == 3
    gtk_widget_show_all(GTK_WIDGET(vbox));
#endif
    g_signal_connect(this->label, "activate-link", G_CALLBACK(+[](GtkLabel*, gchar* uri, gpointer) {
                         XojMsgBox::openURL(nullptr, uri);
                         return true;  // default handler does not work on Windows
                     }),
                     nullptr);
}

LinkPopover::~LinkPopover() {
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

bool LinkPopover::hasLink() const { return (this->link != nullptr); }

std::optional<xoj::util::Rectangle<double>> LinkPopover::getRect() { return rect; }

void LinkPopover::updateRect() {
    if (!this->link) {
        this->rect = std::nullopt;
        return;
    }
    this->rect = this->link->boundingRect();
}

void LinkPopover::updateLabel() {
    if (!this->link) {
        return;
    }
    if (this->markup) {
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
    if (!this->link || !this->rect.has_value()) {
        return;
    }

    XojPageView* pageView = view->getViewFor(view->getCurrentPage());
    auto pos = pageView->getPixelPosition();
    auto q = pageView->getXournal()->getScrollHandling()->getPosition();
    auto zoom = pageView->getZoom();
    auto r = this->rect.value();
    GdkRectangle rect{round_cast<int>(pos.x - q.x + r.x * zoom), round_cast<int>(pos.y - q.y + r.y * zoom),
                      round_cast<int>(r.width * zoom), round_cast<int>(r.height * zoom)};
    gtk_popover_set_pointing_to(this->popover, &rect);
}

void LinkPopover::linkTo(Link* link) {
    this->link = link;
    updateLabel();
    updateRect();
    positionPopover();
}
