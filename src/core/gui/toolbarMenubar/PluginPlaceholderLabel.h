#pragma once

#include <string>
#include <gtk/gtk.h>

#include "AbstractToolItem.h"

struct ToolbarPlaceholderEntry;

class PluginPlaceholderLabel : public AbstractToolItem {
public:
    explicit PluginPlaceholderLabel(ToolbarPlaceholderEntry* t);
    ~PluginPlaceholderLabel() override;

    std::string getToolDisplayName() const override;

    // Extra: allow updating the label text later
    void setText(const std::string& text);

protected:
    xoj::util::WidgetSPtr createItem(bool horizontal) override;
    GtkWidget* getNewToolIcon() const override; // unused but required by base

private:
    ToolbarPlaceholderEntry* t;
    std::vector<GtkWidget*> labelWidgets;
};
