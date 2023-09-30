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
#include <gtk/gtk.h>                // for GtkToolItem, GtkWidget, GtkToolB...

#include "enums/ActionGroup.enum.h"  // for ActionGroup
#include "enums/ActionType.enum.h"   // for ActionType
#include "util/raii/GObjectSPtr.h"

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
    virtual GtkToolItem* createItem(bool horizontal);
    virtual GtkToolItem* createTmpItem(bool horizontal);

    bool isUsed() const;
    void setUsed(bool used);

    static void toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item);

    virtual std::string getToolDisplayName() const = 0;

    /**
     * Returns: (transfer floating)
     */
    virtual GtkWidget* getNewToolIcon() const = 0;

    /**
     * Enable / Disable the tool item
     */
    void enable(bool enabled) override;

    GtkToolItem* getItem() const;

protected:
    virtual GtkToolItem* newItem() = 0;

protected:
    GtkToolItem* item = nullptr;

    bool toolToggleButtonActive = false;
    bool toolToggleOnlyEnable = false;

    /**
     * This item is already somewhere in the toolbar
     */
    bool used = false;
};
