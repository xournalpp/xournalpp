#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar) {
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() = default;

auto SidebarPreviewPageEntry::getRenderType() -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    sidebar->getControl()->getScrollHandler()->scrollToPage(page);
    sidebar->getControl()->firePageSelected(page);
}
