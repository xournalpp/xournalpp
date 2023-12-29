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
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GVariantSPtr.h"

class AbstractToolItem {
public:
    AbstractToolItem(std::string id);
    virtual ~AbstractToolItem();

    AbstractToolItem(AbstractToolItem const&) = delete;
    auto operator=(AbstractToolItem const&) -> AbstractToolItem& = delete;
    AbstractToolItem(AbstractToolItem&&) = delete;                     // Implement if desired
    auto operator=(AbstractToolItem&&) -> AbstractToolItem& = delete;  // Implement if desired

public:
    virtual xoj::util::WidgetSPtr createItem(bool horizontal) = 0;

    xoj::util::WidgetSPtr createToolItem(bool horizontal);

    const std::string& getId() const;
    virtual std::string getToolDisplayName() const = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() const = 0;

protected:
    std::string id;
    std::vector<xoj::util::WidgetSPtr> instances;  // useful?
};
