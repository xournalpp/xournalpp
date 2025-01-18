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

#include <string>  // for string, allocator
#include <vector>

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/Action.enum.h"  // for Action
#include "gui/PopoverFactory.h"
#include "util/raii/GVariantSPtr.h"

class StylePopoverFactory final: public PopoverFactory {
public:
    struct Entry {
        template <typename T>
        Entry(std::string name, std::string icon, T t):
                name(std::move(name)), icon(std::move(icon)), target(xoj::util::makeGVariantSPtr<T>(t)) {}
        template <typename T>
        Entry(std::string name, T t): name(std::move(name)), target(xoj::util::makeGVariantSPtr<T>(t)) {}
        std::string name;
        std::string icon;
        xoj::util::GVariantSPtr target;  /// Target value of the associated GSimpleAction corresponding to the entry
    };

    /// @param styleAction The action activated when an entry in the popover is clicked
    StylePopoverFactory(Action styleAction, std::vector<Entry> entries);
    ~StylePopoverFactory() override = default;

    GtkWidget* createPopover() const override;

private:
    std::vector<Entry> entries;
    /// The action activated when an entry in the popover is clicked
    Action styleAction;
};
