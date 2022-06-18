#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"                                // for Control
#include "control/ScrollHandler.h"                          // for ScrollHan...
#include "gui/sidebar/previews/page/SidebarPreviewPages.h"  // for SidebarPr...

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar) {
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() = default;

auto SidebarPreviewPageEntry::getRenderType() -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    sidebar->getControl()->getScrollHandler()->scrollToPage(page);
    sidebar->getControl()->firePageSelected(page);
}
