/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "AbstractToolItem.h"

struct ToolbarPlaceholderEntry;

class PluginPlaceholderLabel: public AbstractToolItem {
public:
    explicit PluginPlaceholderLabel(ToolbarPlaceholderEntry* t);
    ~PluginPlaceholderLabel() override;

    std::string getToolDisplayName() const override;

    // Extra: allow updating the label text later
    void setText(std::string text);

protected:
    xoj::util::WidgetSPtr createItem(bool horizontal) override;
    GtkWidget* getNewToolIcon() const override;  // unused but required by base

private:
    ToolbarPlaceholderEntry* t;
    std::vector<GtkWidget*> labelWidgets;

    auto sanitizeText(const std::string& text) const -> std::string;
    auto getDisplayText() const -> std::string;
};
