#include "StylePopoverFactory.h"

#include <utility>  // for move

#include <glib.h>
#include <gtk/gtk.h>

#include "util/GtkUtil.h"
#include "util/gtk4_helper.h"


StylePopoverFactory::StylePopoverFactory(Action styleAction, std::vector<Entry> entries):
        entries(std::move(entries)), styleAction(styleAction) {}

GtkWidget* StylePopoverFactory::createPopover() const {
    GtkWidget* popover = gtk_popover_new();
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_popover_set_child(GTK_POPOVER(popover), box);

    std::string actionName = std::string("win.") + Action_toString(styleAction);
    auto appendItem = [&](const StylePopoverFactory::Entry& e) {
        GtkWidget* btn = gtk_check_button_new();
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
        g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkCheckButton*, gpointer popover) {
                                    gtk_popover_popdown(GTK_POPOVER(popover));
                                }),
                                popover, GConnectFlags(0));
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), e.target.get());
#if GTK_CHECK_VERSION(4, 8, 0)
        // gtk_check_button_set_child was added in gtk 4.8.0
        if (!e.icon.empty()) {
            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_box_append(GTK_BOX(hbox), gtk_image_new_from_icon_name(e.icon.c_str()));
            gtk_box_append(GTK_BOX(hbox), gtk_label_new(e.name.c_str()));
            gtk_check_button_set_child(GTK_CHECK_BUTTON(btn), hbox);
        } else {
            gtk_check_button_set_label(GTK_CHECK_BUTTON(btn), e.name.c_str());
        }
#else
        gtk_check_button_set_label(GTK_CHECK_BUTTON(btn), e.name.c_str());
#endif
        gtk_box_append(GTK_BOX(box), btn);
    };

    for (const auto& e: entries) {
        appendItem(e);
    }

    return popover;
}
