/*
 * Xournal++
 *
 * The recent opened files
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <vector>  // for vector

#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget, GtkRecentInfo

#include "filesystem.h"  // for path

class RecentManagerListener {
public:
    virtual ~RecentManagerListener();

    /**
     * This function is called whenever some file
     * from the recent menu is opened
     */
    virtual void fileOpened(fs::path const& file) = 0;
};

/**
 * @brief Handles the GtkMenu displaying the recent files
 */
class RecentManager {
public:
    RecentManager();
    virtual ~RecentManager();

public:
    /**
     * Adds a file to the underlying GtkRecentManager
     * without altering the menu
     */
    static void addRecentFileFilename(const fs::path& filename);

    /**
     * Removes a file from the underlying GtkRecentManager
     * without altering the menu
     */
    [[maybe_unused]] static void removeRecentFileFilename(const fs::path& filename);

    /**
     * Removes all of the menu items corresponding to recent files
     */
    void freeOldMenus();

    /**
     * Updates the menu of recent files
     */
    void updateMenu();

    /**
     * Notifies all RecentManagerListener%s that a new
     * file is opened
     */
    void openRecent(const fs::path& p);

    /**
     * Returns the root menu containing all the items
     * corresponding to the recent files
     */
    GtkWidget* getMenu();

    /**
     * Adds a new RecentManagerListener to be notified
     * of opened files
     */
    void addListener(RecentManagerListener* l);

    /**
     * Returns the most recent xoj item from the underlying GtkRecentManager
     * or nullptr, if no recent files exist
     */
    GtkRecentInfo* getMostRecent();

private:
    void addRecentMenu(GtkRecentInfo* info, int i);

private:
    gulong recentHandlerId{};

    std::vector<RecentManagerListener*> listener;

    GtkWidget* menu;
    std::vector<GtkWidget*> menuItemList;
};
