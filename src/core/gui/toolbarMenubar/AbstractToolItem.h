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

#include <string>  // for string
#include <vector>

#include <gtk/gtk.h>  // for GtkWidget

#include "enums/Action.enum.h"
#include "gui/toolbarMenubar/ToolbarSide.h"
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GVariantSPtr.h"

class AbstractToolItem {
public:
    // If you add a category, don't forget to give it a label in ToolbarCustomizeDialog.cpp.
    // Keep the enum contiguous
    enum class Category : unsigned char {
        FILES,
        TOOLS,
        COLORS,
        NAVIGATION,
        MISC,
        SELECTION,
        AUDIO,
        SEPARATORS,
        PLUGINS,
        ENUMERATOR_COUNT  // Keep last
    };
    AbstractToolItem(std::string id, Category cat);
    virtual ~AbstractToolItem();

    AbstractToolItem(AbstractToolItem const&) = delete;
    auto operator=(AbstractToolItem const&) -> AbstractToolItem& = delete;
    AbstractToolItem(AbstractToolItem&&) = delete;                     // Implement if desired
    auto operator=(AbstractToolItem&&) -> AbstractToolItem& = delete;  // Implement if desired

public:
    struct Widgetry {
        xoj::util::WidgetSPtr item;
        xoj::util::WidgetSPtr proxy;
    };
    virtual Widgetry createItem(ToolbarSide s) = 0;

    const std::string& getId() const;
    Category getCategory() const;

    virtual std::string getToolDisplayName() const = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() const = 0;

protected:
    std::string id;
    const Category category;
};
