/*
 * Xournal++
 *
 * Previews of the pages in the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr
#include <string>   // for string
#include <tuple>    // for tuple
#include <vector>   // for vector

#include <glib.h>     // for gulong
#include <gtk/gtk.h>  // for GtkWidget

#include "gui/IconNameHelper.h"                            // for IconNameHe...
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"  // for SidebarPre...
#include "gui/sidebar/previews/base/SidebarToolbar.h"      // for SidebarAct...

class Control;
class GladeGui;


class SidebarPreviewPages: public SidebarPreviewBase {
public:
    SidebarPreviewPages(Control* control, GladeGui* gui, SidebarToolbar* toolbar);
    ~SidebarPreviewPages() override;

public:
    /**
     * Called when an action is performed
     */
    void actionPerformed(SidebarActions action) override;

    void enableSidebar() override;

    /**
     * @overwrite
     */
    std::string getName() override;

    /**
     * @overwrite
     */
    std::string getIconName() override;

    /**
     * Update the preview images
     * @overwrite
     */
    void updatePreviews() override;

    /**
     * Opens the page preview context menu, at the current cursor position, for
     * the given page.
     */
    void openPreviewContextMenu() override;

public:
    // DocumentListener interface (only the part which is not handled by SidebarPreviewBase)
    void pageSizeChanged(size_t page) override;
    void pageChanged(size_t page) override;
    void pageSelected(size_t page) override;
    void pageInserted(size_t page) override;
    void pageDeleted(size_t page) override;

private:
    /**
     * Unselect the last selected page, if any
     */
    void unselectPage();

    /**
     * The context menu to display when a page is right-clicked.
     */
    GtkWidget* const contextMenu = nullptr;

    GtkWidget* contextMenuMoveUp = nullptr;
    GtkWidget* contextMenuMoveDown = nullptr;

    /**
     * The data passed to the menu item callbacks.
     */
    struct ContextMenuData {
        SidebarToolbar* toolbar;
        SidebarActions actions;
    };


    /**
     * The signals connected to the context menu items. This must be kept track
     * of so the data can be deallocated safely.
     */
    std::vector<std::tuple<GtkWidget*, gulong, std::unique_ptr<ContextMenuData>>> contextMenuSignals;

private:
    IconNameHelper iconNameHelper;
};
