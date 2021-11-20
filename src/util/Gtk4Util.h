#pragma once

#include "gtk/gtk.h"

#if GTK_CHECK_VERSION(4, 6, 0)
#else

inline void gtk_menu_button_set_child(GtkMenuButton* menu_button, GtkWidget* child) {
    GValue g;
    g_value_set_object(&g, child);
    g_object_set_property(G_OBJECT(menu_button), "child", &g);
}

#endif