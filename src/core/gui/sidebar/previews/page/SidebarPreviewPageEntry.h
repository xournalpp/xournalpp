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

#include "gui/sidebar/previews/base/SidebarPreviewBaseEntry.h"
#include "model/PageRef.h"

#include "SidebarPreviewPages.h"

class SidebarPreviewBase;
class SidebarPreviewPages;

class SidebarPreviewPageEntry: public SidebarPreviewBaseEntry {
public:
    SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page);
    ~SidebarPreviewPageEntry() override;

public:
    /**
     * @return What should be rendered
     * @override
     */
    PreviewRenderType getRenderType() override;

protected:
    SidebarPreviewPages* sidebar;
    void mouseButtonPressCallback() override;

private:
    friend class PreviewJob;
};
