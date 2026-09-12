#include "util/GtkUtil.h"

#include <algorithm>

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

std::optional<double> getWidgetDPI(GtkWidget* w) {
    GtkNative* nat = gtk_widget_get_native(w);
    GdkDisplay* disp = gtk_widget_get_display(w);
    if (nat && disp) {
        GdkSurface* surf = gtk_native_get_surface(nat);
        GdkMonitor* mon = surf ? gdk_display_get_monitor_at_surface(disp, surf) : nullptr;
        if (mon) {
            int h = gdk_monitor_get_height_mm(mon);
            GdkRectangle r;
            gdk_monitor_get_geometry(mon, &r);
            auto res = static_cast<double>(r.height) * 25.4 / static_cast<double>(h);
            g_debug("Automatic DPI conf: monitor height %d mm - DPI = %f", h, res);
            // I assume some monitors may not return the right value: clamp to avoid wild values that could cause very
            // bad performances. Not sure what good clamping values should be here.
            // DPI > 720 should use HiDPI scaling but smaller devices (tablet/phone) can have high DPI values
            res = std::clamp(res, 36., 720.);
            return res;
        }
    }
    return std::nullopt;
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
