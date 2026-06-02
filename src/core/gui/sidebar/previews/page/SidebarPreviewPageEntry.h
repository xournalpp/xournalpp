/*
 * Xournal++
 *
 * A Sidebar preview widget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"  // for Previ...
#include "model/PageRef.h"                                      // for PageRef

class SidebarPreviewPages;

class SidebarPreviewPageEntry: public SidebarPreviewBaseEntry {
public:
    SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page, size_t index);
    ~SidebarPreviewPageEntry() override;

public:
    PreviewRenderType getRenderType() const override;

    void setIndex(size_t index);
    size_t getIndex() const;

    bool isSelected() const;
    double getZoom() const;

    GtkWidget* getWidget() const override;

protected:
    SidebarPreviewPages* sidebar;
    void mouseButtonPressCallback() override;

private:
    /// Container for the preview and it potential label
    xoj::util::WidgetSPtr widget;
    /// Label for page numbers (may be nullptr if page numbering is set to none)
    xoj::util::GObjectSPtr<GtkLabel> label;

    size_t index;
    friend class PreviewJob;

    // void drawEntryNumber(cairo_t* cr);
};
