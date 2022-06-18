/*
 * Xournal++
 *
 * Sidebar preview layout
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkButton, GtkToolButton

class GladeGui;


enum SidebarActions {
    SIDEBAR_ACTION_NONE = 0,
    SIDEBAR_ACTION_MOVE_UP = 1 << 0,
    SIDEBAR_ACTION_MOVE_DOWN = 1 << 1,
    SIDEBAR_ACTION_COPY = 1 << 2,
    SIDEBAR_ACTION_DELETE = 1 << 3,
    SIDEBAR_ACTION_NEW_BEFORE = 1 << 4,
    SIDEBAR_ACTION_NEW_AFTER = 1 << 5,
    SIDEBAR_ACTION_MERGE_DOWN = 1 << 6,
};

class SidebarToolbarActionListener {
public:
    virtual ~SidebarToolbarActionListener();

    /**
     * Called when an action is performed
     */
    virtual void actionPerformed(SidebarActions action);
};

class SidebarToolbar {
public:
    SidebarToolbar(SidebarToolbarActionListener* listener, GladeGui* gui);
    virtual ~SidebarToolbar();

public:
    /**
     * Sets the button enabled / disabled
     */
    void setButtonEnabled(SidebarActions enabledActions);
    void setButtonTooltips(const std::string& tipUp, const std::string& tipDown, const std::string& tipCopy,
                           const std::string& tipDelete);

    void setHidden(bool hidden);

    /**
     * Runs the actions directly.
     */
    void runAction(SidebarActions actions);

private:
private:
    static void btUpClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
    static void btDownClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
    static void btCopyClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);
    static void btDeleteClicked(GtkToolButton* toolbutton, SidebarToolbar* toolbar);

private:
    /**
     * Listener for actions
     */
    SidebarToolbarActionListener* listener;

    /**
     * Button move Page up
     */
    GtkButton* btUp;

    /**
     * Button move Page down
     */
    GtkButton* btDown;

    /**
     * Button copy current page
     */
    GtkButton* btCopy;

    /**
     * Button delete page
     */
    GtkButton* btDelete;
};
