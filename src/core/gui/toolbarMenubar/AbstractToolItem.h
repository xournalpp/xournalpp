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

    GtkToolItem* createToolItem(bool horizontal);

    bool isUsed() const;
    /**
     * May be used to clean up data in derived class when an item is no longer used
     * Derived implementations should call AbstractToolItem::setUsed()
     */
    virtual void setUsed(bool used);

    const std::string& getId() const;
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
