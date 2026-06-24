#include "FlyingClickableIcon.h"

#include <gdk/gdk.h>      // for GdkRectangle
#include <glib-object.h>  // for G_CALLBACK, g_signal_con...

#include "util/Assert.h"
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"

#include "MainWindow.h"  // for MainWindow
#include "XournalView.h"


static xoj::util::Point<char> anchorToPoint(FlyingClickableIcon::Anchor a) {
    switch (a) {
        case FlyingClickableIcon::Anchor::WEST:
            return {0, 1};
        case FlyingClickableIcon::Anchor::EAST:
            return {2, 1};
        case FlyingClickableIcon::Anchor::CENTER:
            return {1, 1};
        case FlyingClickableIcon::Anchor::NORTH_WEST:
            return {0, 0};
        case FlyingClickableIcon::Anchor::NORTH_EAST:
            return {2, 0};
        case FlyingClickableIcon::Anchor::NORTH:
            return {1, 0};
        case FlyingClickableIcon::Anchor::SOUTH_WEST:
            return {0, 2};
        case FlyingClickableIcon::Anchor::SOUTH_EAST:
            return {2, 2};
        case FlyingClickableIcon::Anchor::SOUTH:
            return {1, 2};
        default:
            xoj_assert(false);
            return {1, 1};
    }
}

FlyingClickableIcon::FlyingClickableIcon(MainWindow* theMainWindow, const char* icon, Anchor a):
        mainWindow(theMainWindow),
        overlay(GTK_OVERLAY(gtk_widget_get_ancestor(mainWindow->getXournal()->getWidget(), GTK_TYPE_OVERLAY))),
        widget(gtk_button_new_from_icon_name(icon), xoj::util::adopt),
        anchor(anchorToPoint(a)) {
    gtk_widget_add_css_class(widget.get(), "flying-icon");
    gtk_widget_set_can_focus(widget.get(), false);

    // position overlay widgets
    signals.emplace_back(std::make_pair(G_OBJECT(overlay),
                                        g_signal_connect(overlay, "get-child-position",
                                                         xoj::util::wrap_for_g_callback_v<getOverlayPosition>, this)));

    gtk_overlay_add_overlay(overlay, this->widget.get());

#if GTK_MAJOR_VERSION == 3
    gtk_widget_show_all(this->widget.get());
#endif
}

FlyingClickableIcon::~FlyingClickableIcon() {
    gtk_overlay_remove_overlay(this->overlay, this->widget.get());
    for (auto& s: signals) {
        g_signal_handler_disconnect(s.first, s.second);
    }
}

void FlyingClickableIcon::setPosition(xoj::util::Point<int> position) {
    this->position = position;
    if (gtk_widget_get_visible(this->widget.get())) {
        gtk_widget_queue_resize(this->widget.get());  // should trigger the icon to move
    }
}

void FlyingClickableIcon::addSignal(GObject* o, gulong sig) { signals.emplace_back(std::make_pair(o, sig)); }

/**
 * getOverlayPosition - this is how we position the widget
 * The requested location is communicated via the FlyingClickableIcon member variable `position`
 */
auto FlyingClickableIcon::getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                             FlyingClickableIcon* self) -> bool {
    if (self->widget.get() != widget) {
        return false;
    }

    GtkRequisition natural;
    gtk_widget_get_preferred_size(widget, nullptr, &natural);
    allocation->width = natural.width;
    allocation->height = natural.height;
    allocation->x = self->position.x - (self->anchor.x * allocation->width) / 2;  // Center the icon as best we can
    allocation->y = self->position.y - (self->anchor.y * allocation->height) / 2;

    // Then translate into Overlay coordinates
    GtkWidget* scrolledWindow =
            gtk_widget_get_ancestor(self->mainWindow->getXournal()->getWidget(), GTK_TYPE_SCROLLED_WINDOW);
    gtk_widget_translate_coordinates(scrolledWindow, GTK_WIDGET(overlay), allocation->x, allocation->y, &allocation->x,
                                     &allocation->y);

    return true;
}
