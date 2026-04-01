#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"                                // for Control
#include "control/ScrollHandler.h"                          // for ScrollHan...
#include "control/settings/Settings.h"                      // for Settings
#include "gui/PagePreviewDecoration.h"                      // for Drawing  ...
#include "gui/sidebar/previews/page/SidebarPreviewPages.h"  // for SidebarPr...
#include "util/gtk4_helper.h"

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page, size_t index):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar), index(index) {
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() ==
        SidebarNumberingStyle::NUMBER_BELOW_PREVIEW) {
        gtk_widget_set_size_request(this->button.get(), imageWidth, imageHeight + PagePreviewDecoration::MARGIN_BOTTOM);
    }
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() {
    GtkWidget* w = this->getWidget();
    gtk_fixed_remove(GTK_FIXED(gtk_widget_get_parent(w)), w);
}

auto SidebarPreviewPageEntry::getRenderType() const -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    auto* control = sidebar->getControl();
    if (control->getCurrentPageNo() != index) {
        control->getScrollHandler()->jumpToPage(page);
    }
    control->firePageSelected(page);
}

void SidebarPreviewPageEntry::paint(cairo_t* cr) {
    SidebarPreviewBaseEntry::paint(cr);
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() == SidebarNumberingStyle::NONE) {
        return;
    }
    drawEntryNumber(cr);
}

void SidebarPreviewPageEntry::drawEntryNumber(cairo_t* cr) {
    PagePreviewDecoration::drawDecoration(cr, this, this->sidebar->getControl());
}

auto SidebarPreviewPageEntry::getHeight() const -> int {
    if (sidebar->getControl()->getSettings()->getSidebarNumberingStyle() ==
        SidebarNumberingStyle::NUMBER_BELOW_PREVIEW) {
        return imageHeight + PagePreviewDecoration::MARGIN_BOTTOM;
    }
    return imageHeight;
}

void SidebarPreviewPageEntry::setIndex(size_t index) { this->index = index; }

size_t SidebarPreviewPageEntry::getIndex() const { return this->index; }

bool SidebarPreviewPageEntry::isSelected() const { return this->selected; }

double SidebarPreviewPageEntry::getZoom() const { return this->sidebar->getZoom(); }
