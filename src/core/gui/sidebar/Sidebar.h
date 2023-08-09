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
#include <memory>   // for unique_ptr
#include <vector>   // for vector

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
    void addPage(std::unique_ptr<AbstractSidebarPage> page);

    // SidebarToolbarActionListener
public:
    /**
     * Called when an action is performed
     */
    void actionPerformed(SidebarActions action) override;

public:
    /**
     * Layout sidebar
     */
    void layout();

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

    /**
     * Ask the user whether a page with the given id
     * should be added to the document.
     */
    void askInsertPdfPage(size_t pdfPage);

    /**
     * Get how many pages are contained in this sidebar
     */
    size_t getNumberOfPages();

    /**
     * Get index of the currently selected page
     */
    size_t getSelectedPage();

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
    std::vector<std::unique_ptr<AbstractSidebarPage>> pages;

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
    size_t currentPageIdx{0};

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
