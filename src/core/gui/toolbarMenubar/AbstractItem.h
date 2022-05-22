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

#include <string>
#include <vector>

#include <gtk/gtk.h>

#include "control/Actions.h"


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
    virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);

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

    virtual void actionPerformed(ActionType action, ActionGroup group, GdkEvent* event, GtkMenuItem* menuitem,
                                 GtkToolButton* toolbutton, bool selected);

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
