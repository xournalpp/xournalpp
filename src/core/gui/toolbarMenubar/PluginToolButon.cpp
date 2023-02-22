#include <utility>  // for move

#include "PluginToolButton.h"

PluginToolButton::PluginToolButton(ActionHandler* handler, ToolbarButtonEntry* t):
        ToolButton(handler, std::move(t->toolbarId), ACTION_NONE, std::move(t->iconName), std::move(t->description)),
        t(t) {}

PluginToolButton::~PluginToolButton() = default;

auto PluginToolButton::createItem(bool horizontal) -> GtkToolItem* {
    if (this->item) {
        return this->item;
    }

    this->item = createTmpItem(horizontal);
    g_object_ref(this->item);

    // Connect signal
    g_signal_connect(item, "clicked",
                     G_CALLBACK(+[](GtkWidget* bt, ToolbarButtonEntry* te) { te->plugin->executeToolbarButton(te); }),
                     this->t);

    return this->item;
}
