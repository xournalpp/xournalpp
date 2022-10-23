/*
 * Xournal++
 *
 * Abstract Toolbar / Menubar entry
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gdk/gdk.h>  // for GdkEvent
#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget, GtkMenuItem, GtkToolB...

#include "control/Actions.h"         // for ActionHandler (ptr only), Action...
#include "enums/ActionGroup.enum.h"  // for ActionGroup, GROUP_NOGROUP
#include "enums/ActionType.enum.h"   // for ActionType, ACTION_NONE


class AbstractItem: public ActionEnabledListener, public ActionSelectionListener {
public:
    AbstractItem(std::string id, ActionHandler* handler, ActionType action, GtkWidget* menuitem = nullptr);
    ~AbstractItem() override;

    AbstractItem(AbstractItem const&) = delete;
    auto operator=(AbstractItem const&) -> AbstractItem& = delete;
    AbstractItem(AbstractItem&&) = delete;                     // Implement if desired
    auto operator=(AbstractItem&&) -> AbstractItem& = delete;  // Implement if desired

public:
    void actionSelected(ActionGroup group, ActionType action) override;

    /**
     * Override this method
     */
    virtual void selected(ActionGroup group, ActionType action);

    void actionEnabledAction(ActionType action, bool enabled) override;
    virtual void activated(GtkMenuItem* menuitem, GtkToolButton* toolbutton);

    virtual std::string getId() const;

    void setTmpDisabled(bool disabled);
    bool isEnabled() const;

    ActionType getActionType();

    /**
     * Register a menu item. If there is already one registered, the new one will be ignored
     */
    void setMenuItem(GtkWidget* menuitem);

protected:
    virtual void enable(bool enabled);

    virtual void actionPerformed(ActionType action, ActionGroup group, GtkToolButton* toolbutton, bool selected);

private:
protected:
    ActionGroup group = GROUP_NOGROUP;
    ActionType action = ACTION_NONE;

    std::string id;

    ActionHandler* handler = nullptr;

    bool enabled = true;

private:
    gulong menuSignalHandler = 0;
    GtkWidget* menuitem = nullptr;

    /**
     * This is a check menu item which is not displayed as radio
     */
    bool checkMenuItem = false;

    /**
     * ignore event if the menu is programmatically changed
     */
    bool ignoreNextCheckMenuEvent = false;

    /**
     * Keep the state for toggle / radio menu handling
     */
    bool itemActive = false;
};
