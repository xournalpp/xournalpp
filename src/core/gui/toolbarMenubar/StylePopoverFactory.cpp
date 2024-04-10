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

#if GTK_MAJOR_VERSION == 3
    GtkRadioButton* group = nullptr;
    const char* actionName = Action_toString(styleAction);
    auto appendItem = [&](const StylePopoverFactory::Entry& e) {
        GtkWidget* btn = gtk_radio_button_new_from_widget(group);
        group = GTK_RADIO_BUTTON(btn);
        xoj::util::gtk::setRadioButtonActionName(GTK_RADIO_BUTTON(btn), "win", actionName);
        g_signal_connect_object(btn, "clicked", G_CALLBACK(+[](GtkButton*, gpointer popover) {
                                    gtk_popover_popdown(GTK_POPOVER(popover));
                                }),
                                popover, GConnectFlags(0));
#else
    GtkCheckButton* group = nullptr;
    std::string actionName = std::string("win.") + Action_toString(styleAction);
    g_warning("TODO: check is grouping necessary StylePopoverFactory::createPopover()");
    auto appendItem = [&](const StylePopoverFactory::Entry& e) {
        GtkWidget* btn = gtk_check_button_new();
        // Is grouping necessary here? The GTK4 doc is unclear
        gtk_check_button_set_group(GTK_CHECK_BUTTON(btn), std::exchange(group, GTK_CHECK_BUTTON(btn)));
        gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.c_str());
        g_signal_connect_object(btn, "toggled", G_CALLBACK(+[](GtkCheckButton*, gpointer popover){
            gtk_popover_popdown(GTK_POPOVER(popover));}), popover, GConnectFlags(0));
#endif
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), e.target.get());
        if (!e.icon.empty()) {
            GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
            gtk_box_append(GTK_BOX(hbox), gtk_image_new_from_icon_name(e.icon.c_str()));
            gtk_box_append(GTK_BOX(hbox), gtk_label_new(e.name.c_str()));
            gtk_button_set_child(GTK_BUTTON(btn), hbox);
        } else {
            gtk_button_set_label(GTK_BUTTON(btn), e.name.c_str());
        }
        gtk_box_append(GTK_BOX(box), btn);
    };

    for (const auto& e: entries) {
        appendItem(e);
    }

    return popover;
}
