#include <algorithm>
#include "PluginPlaceholderLabel.h"

#include "config-features.h"

#ifdef ENABLE_PLUGINS

#include "plugin/Plugin.h" // for ToolbarPlaceholderEntry
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"
#include <algorithm>
#include <iostream>

PluginPlaceholderLabel::PluginPlaceholderLabel(ToolbarPlaceholderEntry* t)
    : AbstractToolItem(t->toolbarId, Category::PLUGINS), t(t) {
    t->label = this;
}

PluginPlaceholderLabel::~PluginPlaceholderLabel() = default;

auto PluginPlaceholderLabel::createItem(bool) -> xoj::util::WidgetSPtr {
    std::string labelText = t->value;
    if (labelText.empty()) {
        labelText = t->toolbarId;
    }
    // Replace newlines with spaces
    std::replace(labelText.begin(), labelText.end(), '\n', ' ');
    std::replace(labelText.begin(), labelText.end(), '\r', ' ');
    // Truncate and add ... if too long (e.g., > 32 chars)
    constexpr size_t maxLen = 32;
    if (labelText.length() > maxLen) {
        size_t keep = maxLen - 3;
        labelText = labelText.substr(0, keep) + "...";
    }
    xoj::util::WidgetSPtr item(gtk_label_new(labelText.c_str()), xoj::util::adopt);
    this->labelWidget = item.get(); // keep raw pointer for updates
    gtk_widget_set_can_focus(item.get(), false);
    gtk_widget_set_size_request(item.get(), 200, -1);
    return item;
}

void PluginPlaceholderLabel::setText(const std::string& text) {
    if (this->labelWidget) {
        std::string displayText = text;
        if (displayText.empty()) {
            displayText = t->toolbarId;
        }
        // Replace newlines with spaces
        std::replace(displayText.begin(), displayText.end(), '\n', ' ');
        std::replace(displayText.begin(), displayText.end(), '\r', ' ');
        constexpr size_t maxLen = 32;
        if (displayText.length() > maxLen) {
            size_t keep = maxLen - 3;
            displayText = displayText.substr(0, keep) + "...";
        }
        gtk_label_set_text(GTK_LABEL(this->labelWidget), displayText.c_str());
        // Force redraw
        gtk_widget_queue_draw(this->labelWidget);
    }
}

auto PluginPlaceholderLabel::getToolDisplayName() const -> std::string {
    return this->t->description;
}

// Placeholders don’t really have icons
auto PluginPlaceholderLabel::getNewToolIcon() const -> GtkWidget* {
    // Return an empty label for the icon in customization dialog
    return gtk_image_new();
}

#endif /* ENABLE_PLUGINS */
