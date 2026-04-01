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
    /// Constructor: links placeholder entry to this label
    explicit PluginPlaceholderLabel(ToolbarPlaceholderEntry* t);

    /// Destructor: disconnects signal handlers and clears label widget vector
    ~PluginPlaceholderLabel() override;

    /// Returns the display name for the placeholder tool
    std::string getToolDisplayName() const override;

    /// Updates all label widgets with new text
    void setText(std::string text);

protected:
    /// Creates a label widget for the placeholder; stores and tracks it
    xoj::util::WidgetSPtr createItem(bool horizontal) override;

    /// Returns a theme-supported icon for the placeholder tool
    GtkWidget* getNewToolIcon() const override;

private:
    ToolbarPlaceholderEntry* t;
    std::vector<GtkWidget*> labelWidgets;

    auto sanitizeText(const std::string& text) const -> std::string;

    /// Gets the display text, using toolbarId as fallback if value is empty
    auto getDisplayText() const -> std::string;
};
