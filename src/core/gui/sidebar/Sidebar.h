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

#include "model/DocumentChangeType.h"                  // for DocumentChange...
#include "model/DocumentListener.h"                    // for DocumentListener

class AbstractSidebarPage;
class Control;
class GladeGui;
class SidebarTabButton;

class Sidebar: public DocumentListener {
public:
    Sidebar(GladeGui* gui, Control* control);
    ~Sidebar() override;

private:
    void initTabs(GtkWidget* sidebarContents);
    void addTab(std::unique_ptr<AbstractSidebarPage> tab);

public:
    /// Update the way the numbers are displayed depending on the value in Settings.
    void updatePageNumberingStyle();

    /**
     * A page was selected, so also select this page in the sidebar
     */
    void selectPageNr(size_t page, size_t pdfPage);

    Control* getControl();

    /**
     * Temporary disable Sidebar (e.g. while saving)
     */
    void setTmpDisabled(bool disabled);

    /**
     * Saves the current size to the settings
     */
    void saveSize();

    /**
     * Get how many pages are contained in this sidebar
     *
     * Todo: it makes no sense to expose this, as the caller has no way of knowing what those pages correspond to
     */
    [[deprecated]] size_t getNumberOfTabs() const;

    /**
     * Get index of the currently selected page
     *
     * Todo: it makes no sense to expose this, as the caller has no way of knowing what this number corresponds to
     */
    [[deprecated]] size_t getSelectedTab() const;

    /**
     * Sets the current selected tab
     *
     * Todo: this should be private
     */
    void setSelectedTab(size_t tab);

public:
    // DocumentListener interface
    void documentChanged(DocumentChangeType type) override;

private:
    /**
     * Page selected
     */
    static void buttonClicked(GtkButton* button, SidebarTabButton* buttonData);

    /**
     * Show/hide tabs based on whether they have content. Select first active tab (page).
     */
    void updateVisibleTabs();

private:
    Control* control = nullptr;

    /**
     * The sidebar pages
     */
    std::vector<std::unique_ptr<AbstractSidebarPage>> tabs;

    /**
     * The bar with the tab selection
     */
    GtkBox* tbSelectTab = nullptr;

    /**
     * The close button of the sidebar
     */
    GtkWidget* buttonCloseSidebar = nullptr;

    /**
     * The current visible page in the sidebar
     */
    GtkWidget* visibleTab = nullptr;

    /**
     * Current active page
     */
    size_t currentTabIdx{0};

    /**
     * The sidebarContents widget
     */
    GtkWidget* sidebarContents = nullptr;
};

class SidebarTabButton {
public:
    SidebarTabButton(Sidebar* sidebar, size_t index, AbstractSidebarPage* page);

public:
    Sidebar* sidebar = nullptr;
    size_t index = 0;
    AbstractSidebarPage* page = nullptr;
};
