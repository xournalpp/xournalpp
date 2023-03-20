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
    /**
     * @return What should be rendered
     * @override
     */
    PreviewRenderType getRenderType() override;

    void setIndex(size_t index);
    size_t getIndex() const;

    bool isSelected() const;
    double getZoom() const;

protected:
    SidebarPreviewPages* sidebar;
    void mouseButtonPressCallback() override;
    void paint(cairo_t* cr) override;
    int getWidgetHeight() override;

private:
    size_t index;
    friend class PreviewJob;

    void drawEntryNumber(cairo_t* cr);
};
