/*
 * Xournal++
 *
 * Base class for prviews in the sidebar
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

#include "gui/GladeGui.h"
#include "gui/sidebar/AbstractSidebarPage.h"

#include "XournalType.h"

class PdfCache;
class SidebarLayout;
class SidebarPreviewBaseEntry;
class SidebarToolbar;

class SidebarPreviewBase: public AbstractSidebarPage {
public:
    SidebarPreviewBase(Control* control, GladeGui* gui, SidebarToolbar* toolbar);
    virtual ~SidebarPreviewBase();

public:
    virtual void enableSidebar();
    virtual void disableSidebar();

    /**
     * Layout the pages to the current size of the sidebar
     */
    void layout();

    /**
     * Update the preview images
     */
    virtual void updatePreviews() = 0;

    /**
     * @overwrite
     */
    virtual bool hasData();

    /**
     * @overwrite
     */
    virtual GtkWidget* getWidget();

    /**
     * Gets the zoom factor for the previews
     */
    double getZoom() const;

    /**
     * Gets the PDF cache for preview rendering
     */
    PdfCache* getCache();

public:
    // DocumentListener interface (only the part handled by SidebarPreviewBase)
    virtual void documentChanged(DocumentChangeType type);
    virtual void pageInserted(size_t page);
    virtual void pageDeleted(size_t page);

protected:
    /**
     * Timeout callback to scroll to a page
     */
    static bool scrollToPreview(SidebarPreviewBase* sidebar);

    /**
     * The size of the sidebar has chnaged
     */
    static void sizeChanged(GtkWidget* widget, GtkAllocation* allocation, SidebarPreviewBase* sidebar);

private:
    /**
     * The scrollbar with the icons
     */
    GtkWidget* scrollPreview = nullptr;

    /**
     * The Zoom of the previews
     */
    double zoom = 0.15;

    /**
     * For preview rendering
     */
    PdfCache* cache = nullptr;

    /**
     * The layouting class for the prviews
     */
    SidebarLayout* layoutmanager = nullptr;


    // Members also used by subclasses
protected:
    /**
     * The currently selected entry in the sidebar, starting from 0
     * -1 means no valid selection
     */
    size_t selectedEntry = -1;

    /**
     * The widget within the scrollarea with the page icons
     */
    GtkWidget* iconViewPreview = nullptr;

    /**
     * The previews
     */
    vector<SidebarPreviewBaseEntry*> previews;

    /**
     * The sidebar is enabled
     */
    bool enabled = false;

    friend class SidebarLayout;
};
