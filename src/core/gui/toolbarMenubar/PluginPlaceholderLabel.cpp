#include "PluginPlaceholderLabel.h"
#include "plugin/Plugin.h" // for ToolbarPlaceholderEntry
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"  // for gtk_popover_new
#include "config-features.h"

#include <algorithm>  // for std::replace
#include <iostream>

#ifdef ENABLE_PLUGINS

/// Constructor: links placeholder entry to this label
PluginPlaceholderLabel::PluginPlaceholderLabel(ToolbarPlaceholderEntry* t)
    : AbstractToolItem(t->toolbarId, Category::PLUGINS), t(t) {
    t->label = this;
}

/// Destructor: disconnects signal handlers and clears label widget vector
PluginPlaceholderLabel::~PluginPlaceholderLabel() {
    for (auto* label : labelWidgets) {
        if (GTK_IS_WIDGET(label)) {
            g_signal_handlers_disconnect_by_data(label, this);
        }
    }
    labelWidgets.clear();
}

/// Creates a label widget for the placeholder; stores and tracks it
auto PluginPlaceholderLabel::createItem(bool) -> xoj::util::WidgetSPtr {
    std::string labelText = t->value;
    if (labelText.empty()) {
        labelText = t->toolbarId;
    }
    std::replace(labelText.begin(), labelText.end(), '\n', ' '); // sanitize newlines
    std::replace(labelText.begin(), labelText.end(), '\r', ' '); // sanitize carriage returns
    constexpr size_t maxLen = 32;
    if (labelText.length() > maxLen) {
        size_t keep = maxLen - 3;
        labelText = labelText.substr(0, keep) + "...";
    }
    xoj::util::WidgetSPtr item(gtk_label_new(labelText.c_str()), xoj::util::adopt);
    GtkWidget* rawLabel = item.get();
    labelWidgets.push_back(rawLabel);
    gtk_widget_set_can_focus(rawLabel, false);
    gtk_widget_set_size_request(rawLabel, 200, -1);
    // Remove label from vector when destroyed
    g_signal_connect(rawLabel, "destroy", G_CALLBACK(+[](GtkWidget* widget, gpointer user_data) {
        auto self = static_cast<PluginPlaceholderLabel*>(user_data);
        auto& widgets = self->labelWidgets;
        widgets.erase(std::remove(widgets.begin(), widgets.end(), widget), widgets.end());
    }), this);
    return item;
}

/// Updates all label widgets with new text
void PluginPlaceholderLabel::setText(const std::string& text) {
    std::string displayText = text;
    if (displayText.empty()) {
        displayText = t->toolbarId;
    }
    std::replace(displayText.begin(), displayText.end(), '\n', ' '); // sanitize newlines
    std::replace(displayText.begin(), displayText.end(), '\r', ' '); // sanitize carriage returns
    constexpr size_t maxLen = 32;
    if (displayText.length() > maxLen) {
        size_t keep = maxLen - 3;
        displayText = displayText.substr(0, keep) + "...";
    }
    for (auto* label : labelWidgets) {
        if (GTK_IS_WIDGET(label)) {
            gtk_label_set_text(GTK_LABEL(label), displayText.c_str());
            gtk_widget_queue_draw(label);
        }
    }
}

/// Returns the display name for the placeholder tool
auto PluginPlaceholderLabel::getToolDisplayName() const -> std::string {
    return this->t->description;
}

/// Returns a theme-supported icon for the placeholder tool
auto PluginPlaceholderLabel::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name("insert-text", GTK_ICON_SIZE_LARGE_TOOLBAR);
}

#endif /* ENABLE_PLUGINS */
