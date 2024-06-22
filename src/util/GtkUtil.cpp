#include "util/GtkUtil.h"

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/Point.h"

namespace xoj::util::gtk {
void setWidgetFollowActionEnabled(GtkWidget* w, GAction* a) {
    g_signal_connect_object(a, "notify::enabled", G_CALLBACK(+[](GObject* a, GParamSpec*, gpointer w) {
                                bool b = g_action_get_enabled(G_ACTION(a));
                                gtk_widget_set_sensitive(GTK_WIDGET(w), b);
                            }),
                            w, GConnectFlags(0));
    gtk_widget_set_sensitive(w, g_action_get_enabled(a));
}

xoj::util::Point<double> gdkSurfaceToWidgetCoordinates(xoj::util::Point<double> p, GtkWidget* w) {
    xoj::util::Point<double> q;
    GtkWidget* window = gtk_widget_get_ancestor(w, GTK_TYPE_WINDOW);
    gtk_native_get_surface_transform(GTK_NATIVE(window), &q.x, &q.y);
    p -= q;

    auto zero = GRAPHENE_POINT_INIT_ZERO;
    auto r = GRAPHENE_POINT_INIT_ZERO;
    if (!gtk_widget_compute_point(window, w, &zero, &r)) {
        xoj_assert(false);  // Widget and its GtkWindow ancestor are related, so this should always succeed
    }
    p += {r.x, r.y};
    return p;
}

};  // namespace xoj::util::gtk
