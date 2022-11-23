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

#include <gio/gio.h>  // for GMenu
#include <glib.h>     // for gulong
#include <gtk/gtk.h>

#include "control/RecentManager.h"
#include "util/raii/GObjectSPtr.h"

#include "AbstractSubmenu.h"

class Control;
class Menubar;

/**
 * @brief Handles the GtkMenu displaying the recent files
 */
class RecentDocumentsSubmenu: public Submenu {
public:
    RecentDocumentsSubmenu(Control* control, GtkApplicationWindow* win);
    virtual ~RecentDocumentsSubmenu();

    /**
     * Updates the menu of recent files
     */
    void updateMenu();

    void setDisabled(bool disabled) override;
    void addToMenubar(MainWindow* win) override;

    static constexpr auto SUBMENU_ID = "menuFileRecent";
    static constexpr auto G_ACTION_NAMESPACE = "win.";

    static constexpr auto OPEN_ACTION_NAME = "open-file-at";

    /**
     * @brief For a disable placeholder saying "No recent files"
     *  To disable a GMenu entry, give it an action name that does not correspond to any GAction
     */
    static constexpr auto DISABLED_ACTION_NAME = "always-disabled-action";

    static constexpr auto CLEAR_LIST_ACTION_NAME = "clear-recent-files";

private:
    static void openFileCallback(GSimpleAction* ga, GVariant* parameter, RecentDocumentsSubmenu* self);

    gulong recentHandlerId{};

    TinyVector<fs::path, RecentManager::MAX_RECENT> xoppFiles;
    TinyVector<fs::path, RecentManager::MAX_RECENT> pdfFiles;
    Control* control;

    /**
     * Keep one menu for each section!
     * Using a single menu with two sections does not work: when adding this to the suitable submenu in the main
     * menubar, it would give
     * <submenu>
     *  <section>
     *   <section>
     *    ...
     *   </section>
     *   <section>
     *    ...
     *   </section>
     *  </section>
     * </submenu>
     * and the section separator does not appear in this case...
     */
    xoj::util::GObjectSPtr<GMenu> menuXoppFiles;
    xoj::util::GObjectSPtr<GMenu> menuPdfFiles;
    xoj::util::GObjectSPtr<GSimpleAction> openFileAction;
    xoj::util::GObjectSPtr<GSimpleAction> clearListAction;
};
