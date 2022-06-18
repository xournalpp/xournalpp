/*
 * Xournal++
 *
 * The Sidebar
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <list>     // for list

#include <gtk/gtk.h>  // for GtkWidget, Gtk...

#include "gui/sidebar/previews/base/SidebarToolbar.h"  // for SidebarActions
#include "model/DocumentChangeType.h"                  // for DocumentChange...
#include "model/DocumentListener.h"                    // for DocumentListener

class AbstractSidebarPage;
class Control;
class GladeGui;
class SidebarPageButton;

class Sidebar: public DocumentListener, public SidebarToolbarActionListener {
public:
    Sidebar(GladeGui* gui, Control* control);
    ~Sidebar() override;

private:
    void initPages(GtkWidget* sidebarContents, GladeGui* gui);
    void addPage(AbstractSidebarPage* page);

    // SidebarToolbarActionListener
public:
    /**
     * Called when an action is performed
     */
    void actionPerformed(SidebarActions action) override;

public:
    /**
     * A page was selected, so also select this page in the sidebar
     */
    void selectPageNr(size_t page, size_t pdfPage);

    Control* getControl();

    /**
     * Sets the current selected page
     */
    void setSelectedPage(size_t page);

    /**
     * Show/hide tabs based on whether they have content. Select first active tab (page).
     */
    void updateVisibleTabs();

    /**
     * Temporary disable Sidebar (e.g. while saving)
     */
    void setTmpDisabled(bool disabled);

    /**
     * Saves the current size to the settings
     */
    void saveSize();

    /**
     * Gets the sidebar toolbar
     */
    SidebarToolbar* getToolbar();

public:
    // DocumentListener interface
    void documentChanged(DocumentChangeType type) override;

private:
    /**
     * Page selected
     */
    static void buttonClicked(GtkToolButton* toolbutton, SidebarPageButton* buttonData);

private:
    Control* control = nullptr;

    GladeGui* gui = nullptr;

    /**
     * The sidebar pages
     */
    std::list<AbstractSidebarPage*> pages;

    /**
     * The Toolbar with the pages
     */
    GtkToolbar* tbSelectPage = nullptr;

    /**
     * The close button of the sidebar
     */
    GtkWidget* buttonCloseSidebar = nullptr;

    /**
     * The current visible page in the sidebar
     */
    GtkWidget* visiblePage = nullptr;

    /**
     * Current active page
     */
    AbstractSidebarPage* currentPage = nullptr;

    /**
     * The sidebarContents widget
     */
    GtkWidget* sidebarContents = nullptr;

    /**
     * Sidebar toolbar
     */
    SidebarToolbar toolbar;
};

class SidebarPageButton {
public:
    SidebarPageButton(Sidebar* sidebar, int index, AbstractSidebarPage* page);

public:
    Sidebar* sidebar = nullptr;
    int index = 0;
    AbstractSidebarPage* page = nullptr;
};
