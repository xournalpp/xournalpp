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
    virtual GtkWidget* createItem(bool horizontal) = 0;

    bool isUsed() const;
    void setUsed(bool used);

    virtual std::string getId() const;
    virtual std::string getToolDisplayName() const = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() const = 0;

    GtkWidget* getItem() const;

protected:
    std::string id;
    xoj::util::WidgetSPtr item;

    /**
     * This item is already somewhere in the toolbar
     */
    bool used = false;
};
