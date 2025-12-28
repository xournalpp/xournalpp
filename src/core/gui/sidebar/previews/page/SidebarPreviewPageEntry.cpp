#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"                                // for Control
#include "control/ScrollHandler.h"                          // for ScrollHan...
#include "control/settings/Settings.h"                      // for Settings

#include "SidebarPreviewPages.h"

static constexpr auto CSS_CLASS_CIRCULAR_BACKGROUND = "page-number-circular";
static constexpr auto CSS_CLASS_SQUARE_BACKGROUND = "page-number-square";
static constexpr auto CSS_CLASS_NUMBER_BELOW = "page-number-below";

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page, size_t index):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar), index(index) {
    auto numberingStyle = sidebar->getControl()->getSettings()->getSidebarNumberingStyle();

    if (numberingStyle == SidebarNumberingStyle::NONE) {
        this->widget = this->button;
        return;
    }

    GtkWidget* lbl = gtk_label_new(std::to_string(index + 1).c_str());
    this->label.reset(GTK_LABEL(lbl), xoj::util::adopt);

    if (numberingStyle == SidebarNumberingStyle::NUMBER_BELOW_PREVIEW) {
        gtk_widget_set_halign(lbl, GTK_ALIGN_CENTER);
        this->widget.reset(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2), xoj::util::adopt);
        gtk_box_append(GTK_BOX(widget.get()), button.get());
        gtk_box_append(GTK_BOX(widget.get()), lbl);
        gtk_widget_add_css_class(lbl, CSS_CLASS_NUMBER_BELOW);
        return;
    }

    this->widget.reset(gtk_overlay_new(), xoj::util::adopt);
    gtk_overlay_set_child(GTK_OVERLAY(widget.get()), button.get());
    gtk_overlay_add_overlay(GTK_OVERLAY(widget.get()), lbl);
    gtk_widget_set_halign(lbl, GTK_ALIGN_END);
    gtk_widget_set_valign(lbl, GTK_ALIGN_END);
    gtk_widget_set_can_target(lbl, false);  // So that clicking on the label still selects the page
    if (numberingStyle == SidebarNumberingStyle::NUMBER_WITH_CIRCULAR_BACKGROUND) {
        gtk_widget_add_css_class(lbl, CSS_CLASS_CIRCULAR_BACKGROUND);
    } else {
        gtk_widget_add_css_class(lbl, CSS_CLASS_SQUARE_BACKGROUND);
    }
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() {
    GtkWidget* w = this->getWidget();
    gtk_fixed_remove(GTK_FIXED(gtk_widget_get_parent(w)), w);
}

auto SidebarPreviewPageEntry::getRenderType() const -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    sidebar->getControl()->getScrollHandler()->scrollToPage(page);
    sidebar->getControl()->firePageSelected(page);
}

void SidebarPreviewPageEntry::setIndex(size_t index) {
    this->index = index;
    if (label) {
        gtk_label_set_text(label.get(), std::to_string(index + 1).c_str());
    }
}

size_t SidebarPreviewPageEntry::getIndex() const { return this->index; }

auto SidebarPreviewPageEntry::getWidget() const -> GtkWidget* { return this->widget.get(); }

bool SidebarPreviewPageEntry::isSelected() const { return this->selected; }

double SidebarPreviewPageEntry::getZoom() const { return this->sidebar->getZoom(); }
