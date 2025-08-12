// TextPlaceholderToolItem.h
#pragma once
#include "gui/toolbarMenubar/AbstractToolItem.h"
#include "tools/TextPlaceholderTool.h"
#include <gtk/gtk.h>


class TextPlaceholderToolItem : public AbstractToolItem {
public:
    TextPlaceholderToolItem(const std::string& id, TextPlaceholderTool* tool)
        : AbstractToolItem(id, Category::PLUGINS), tool(tool), button(nullptr) {}

    static constexpr size_t MAX_LABEL_LENGTH = 30;

    xoj::util::WidgetSPtr createItem(bool /*horizontal*/) override {
        button = gtk_button_new_with_label(tool->getDisplayText().c_str());
        gtk_widget_set_tooltip_text(button, id.c_str());
        gtk_widget_set_sensitive(button, false);
        gtk_widget_set_hexpand(button, TRUE); // Allow button to expand horizontally
        gtk_widget_set_size_request(button, 150, -1); // Increased minimum width
        // Set ellipsization on the label inside the button
        GtkWidget* label = gtk_bin_get_child(GTK_BIN(button));
        if (GTK_IS_LABEL(label)) {
            gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        }
        return xoj::util::WidgetSPtr(GTK_WIDGET(button), xoj::util::adopt);
    }

    std::string getToolDisplayName() const override {
        return id;
    }

    GtkWidget* getNewToolIcon() const override {
        return nullptr;
    }

    void updateLabel() {
        if (button) {
            std::string newLabel = tool->getDisplayText();
            // Truncate long labels and add ellipsis
            if (newLabel.length() > MAX_LABEL_LENGTH) {
                newLabel = newLabel.substr(0, MAX_LABEL_LENGTH - 3) + "...";
            }
            gtk_button_set_label(GTK_BUTTON(button), newLabel.c_str());
        }
    }

private:
    TextPlaceholderTool* tool;
    GtkWidget* button;
};
