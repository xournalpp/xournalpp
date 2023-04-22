#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"                                // for Control
#include "control/ScrollHandler.h"                          // for ScrollHan...
#include "control/settings/Settings.h"                      // for Settings
#include "gui/PagePreviewDecoration.h"                      // for Drawing  ...
#include "gui/sidebar/previews/page/SidebarPreviewPages.h"  // for SidebarPr...

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page, size_t index):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar), index(index) {}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() = default;

auto SidebarPreviewPageEntry::getRenderType() -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    sidebar->getControl()->getScrollHandler()->scrollToPage(page);
    sidebar->getControl()->firePageSelected(page);
}

void SidebarPreviewPageEntry::paint(cairo_t* cr) {
    SidebarPreviewBaseEntry::paint(cr);
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() == SidebarNumberingStyle::NONE) {
        return;
    }
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() ==
        SidebarNumberingStyle::NUMBER_BELOW_PREVIEW) {
        gtk_widget_set_size_request(this->widget, getWidgetWidth(), getWidgetHeight());
    }
    drawEntryNumber(cr);
}

void SidebarPreviewPageEntry::drawEntryNumber(cairo_t* cr) {
    this->drawingMutex.lock();
    PagePreviewDecoration::drawDecoration(cr, this, this->sidebar->getControl());
    this->drawingMutex.unlock();
}

int SidebarPreviewPageEntry::getWidgetHeight() {
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() ==
        SidebarNumberingStyle::NUMBER_BELOW_PREVIEW) {
        return SidebarPreviewBaseEntry::getWidgetHeight() + PagePreviewDecoration::MARGIN_BOTTOM;
    }
    return SidebarPreviewBaseEntry::getWidgetHeight();
}

void SidebarPreviewPageEntry::setIndex(size_t index) { this->index = index; }

size_t SidebarPreviewPageEntry::getIndex() const { return this->index; }

bool SidebarPreviewPageEntry::isSelected() const { return this->selected; }

double SidebarPreviewPageEntry::getZoom() const { return this->sidebar->getZoom(); }