/*
 * Xournal++
 *
 * header for missing gtk4 functions (part of the gtk4 port)
 * will be removed later
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>


void gtk_box_append(GtkBox* box, GtkWidget* child) {
    constexpr auto default_expand = false;
    gtk_box_pack_start(GTK_BOX(box), child, default_expand, true, 0);
}

void gtk_box_remove(GtkBox* box, GtkWidget* child) { gtk_container_remove(GTK_CONTAINER(box), child); }