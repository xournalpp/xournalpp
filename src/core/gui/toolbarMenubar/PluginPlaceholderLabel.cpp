#include "PluginPlaceholderLabel.h"

#include <algorithm>  // for std::replace

#include "plugin/Plugin.h"  // for ToolbarPlaceholderEntry
#include "util/glib_casts.h"
#include "util/gtk4_helper.h"  // for gtk_popover_new

#include "config-features.h"

#ifdef ENABLE_PLUGINS

PluginPlaceholderLabel::PluginPlaceholderLabel(ToolbarPlaceholderEntry* t):
        AbstractToolItem(t->toolbarId, Category::PLUGINS), t(t) {
    t->label = this;
}

PluginPlaceholderLabel::~PluginPlaceholderLabel() {
    for (auto* label: labelWidgets) {
        if (GTK_IS_WIDGET(label)) {
            g_signal_handlers_disconnect_by_data(label, this);
        }
    }
    labelWidgets.clear();
}

auto PluginPlaceholderLabel::sanitizeText(const std::string& text) const -> std::string {
    std::string sanitized = text;
    std::replace(sanitized.begin(), sanitized.end(), '\n', ' ');
    std::replace(sanitized.begin(), sanitized.end(), '\r', ' ');
    return sanitized;
}

auto PluginPlaceholderLabel::getDisplayText() const -> std::string {
    std::string text = t->value;
    if (text.empty()) {
        text = getDisplayId(t->toolbarId);
    }
    return sanitizeText(text);
}

auto PluginPlaceholderLabel::createItem(bool) -> xoj::util::WidgetSPtr {
    const std::string labelText = getDisplayText();

    xoj::util::WidgetSPtr item(gtk_label_new(labelText.c_str()), xoj::util::adopt);
    GtkWidget* rawLabel = item.get();
    labelWidgets.push_back(rawLabel);
    gtk_widget_set_can_focus(rawLabel, false);
    gtk_widget_set_size_request(rawLabel, 200, -1);
    gtk_label_set_ellipsize(GTK_LABEL(rawLabel), PANGO_ELLIPSIZE_END);

    // Remove label from vector when destroyed
    g_signal_connect(rawLabel, "destroy", G_CALLBACK(+[](GtkWidget* widget, gpointer user_data) {
                         auto self = static_cast<PluginPlaceholderLabel*>(user_data);
                         auto& widgets = self->labelWidgets;
                         widgets.erase(std::remove(widgets.begin(), widgets.end(), widget), widgets.end());
                     }),
                     this);
    return item;
}

void PluginPlaceholderLabel::setText(std::string text) {
    t->value = std::move(text);
    const std::string displayText = getDisplayText();

    for (auto* label: labelWidgets) {
        if (GTK_IS_WIDGET(label)) {
            gtk_label_set_text(GTK_LABEL(label), displayText.c_str());
            gtk_widget_queue_draw(label);
        }
    }
}

auto PluginPlaceholderLabel::getToolDisplayName() const -> std::string { return this->t->description; }

auto PluginPlaceholderLabel::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name("dialog-information", GTK_ICON_SIZE_LARGE_TOOLBAR);
}

#endif /* ENABLE_PLUGINS */
