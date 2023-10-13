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

#include <gio/gio.h>
#include <gtk/gtk.h>

#include "control/actions/ActionRef.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

#include "AbstractToolItem.h"  // for AbstractToolItem

class ActionDatabase;

class ComboToolButton: public AbstractToolItem {
public:
    struct Entry {
        template <typename T>
        Entry(std::string name, std::string icon, T t):
                name(std::move(name)), icon(std::move(icon)), target(xoj::util::makeGVariantSPtr<T>(t)) {}
        std::string name;
        std::string icon;
        xoj::util::GVariantSPtr target;  /// Target value of the associated GSimpleAction corresponding to the entry
    };
    /**
     * @brief When all entries correspond to a single action (but different target values)
     * @param iconName Icon used in the toolbar customization dialog to represent this combo button.
     * @param description Description used in the toolbar customization dialog to explain this combo button.
     * @param entries Entries of the combo menu.
     */
    ComboToolButton(std::string id, std::string iconName, std::string description, std::vector<Entry> entries,
                    ActionRef gAction);

    ~ComboToolButton() override = default;

public:
    std::string getToolDisplayName() const override;

protected:
    GtkWidget* createItem(bool horizontal) override;

    GtkWidget* getNewToolIcon() const override;

protected:
    xoj::util::WidgetSPtr popover;
    xoj::util::WidgetSPtr button;
    std::vector<Entry> entries;
    ActionRef gAction;
    std::string iconName;
    std::string description;
};
