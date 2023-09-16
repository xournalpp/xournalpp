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

#include <gdk-pixbuf/gdk-pixbuf.h>  // for GdkPixbuf
#include <gio/gio.h>
#include <gtk/gtk.h>  // for GtkToolItem, GtkWidget, GtkToolB...

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType
#include "util/raii/GObjectSPtr.h"
#include "util/raii/GVariantSPtr.h"

#include "AbstractItem.h"  // for AbstractItem

class ActionHandler;

class AbstractToolItem: public AbstractItem {
public:
    AbstractToolItem(std::string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem = nullptr);

    AbstractToolItem(std::string id);
    ~AbstractToolItem() override;

    AbstractToolItem(AbstractToolItem const&) = delete;
    auto operator=(AbstractToolItem const&) -> AbstractToolItem& = delete;
    AbstractToolItem(AbstractToolItem&&) = delete;                     // Implement if desired
    auto operator=(AbstractToolItem&&) -> AbstractToolItem& = delete;  // Implement if desired

public:
    void selected(ActionGroup group, ActionType action) override;
    virtual GtkWidget* createItem(bool horizontal) = 0;

    GtkToolItem* createToolItem(bool horizontal);

    bool isUsed() const;
    /**
     * May be used to clean up data in derived class when an item is no longer used
     * Derived implementations should call AbstractToolItem::setUsed()
     */
    virtual void setUsed(bool used);

    static void toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item);

    virtual std::string getToolDisplayName() const = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() const = 0;

    GtkWidget* getItem() const;

protected:
    xoj::util::WidgetSPtr item;

    bool toolToggleButtonActive = false;
    bool toolToggleOnlyEnable = false;

    /**
     * This item is already somewhere in the toolbar
     */
    bool used = false;
};
