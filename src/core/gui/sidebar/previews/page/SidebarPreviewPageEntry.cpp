#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar) {
    const auto clickCallback = G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, SidebarPreviewPageEntry* self) {
        // Open context menu on right mouse click
        if (gdk_event_get_event_type(event) == GDK_BUTTON_PRESS) {
            if (gdk_button_event_get_button(event) == 3) {
                self->mouseButtonPressCallback();
                self->sidebar->openPreviewContextMenu();
                return true;
            }
        }
        return false;
    });
    g_signal_connect_after(this->widget, "button-press-event", clickCallback, this);
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() = default;

auto SidebarPreviewPageEntry::getRenderType() -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    sidebar->getControl()->getScrollHandler()->scrollToPage(page);
    sidebar->getControl()->firePageSelected(page);
}
