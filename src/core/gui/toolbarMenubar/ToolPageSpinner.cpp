#include "ToolPageSpinner.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref_sink

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "gui/widgets/SpinPageAdapter.h"          // for SpinPageAdapter
#include "util/Assert.h"                          // for xoj_assert
#include "util/gtk4_helper.h"                     // for gtk_box_append
#include "util/i18n.h"                            // for FS, _, _F, C_F
#include "util/safe_casts.h"                      // for as_signed

ToolPageSpinner::ToolPageSpinner(std::string id, IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id)),
        pageSpinner(std::make_unique<SpinPageAdapter>()),
        iconNameHelper(iconNameHelper) {}

ToolPageSpinner::~ToolPageSpinner() = default;

auto ToolPageSpinner::getPageSpinner() const -> SpinPageAdapter* { return pageSpinner.get(); }

void ToolPageSpinner::setPageInfo(const size_t pageCount, const size_t pdfPage) {
    this->pageCount = pageCount;
    this->pdfPage = pdfPage;
    if (this->lbPageNo) {
        updateLabels();
    }
}

void ToolPageSpinner::updateLabels() {
    std::string ofString = FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}") % as_signed(this->pageCount));
    if (this->pdfPage > 0) {
        if (this->orientation == GTK_ORIENTATION_HORIZONTAL) {
            ofString += std::string(", ") + FS(_F("PDF Page {1}") % as_signed(this->pdfPage));
        } else {
            ofString += std::string("\n") + FS(_F("PDF {1}") % as_signed(this->pdfPage));
        }
        gtk_label_set_text(GTK_LABEL(lbPageNo.get()), ofString.c_str());
    }
}

auto ToolPageSpinner::getToolDisplayName() const -> std::string { return _("Page number"); }

auto ToolPageSpinner::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconNameHelper.iconName("page-spinner").c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolPageSpinner::createItem(bool horizontal) -> GtkWidget* {
    this->orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
    GtkWidget* spinner = gtk_spin_button_new_with_range(0, 1, 1);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(spinner), orientation);
    this->pageSpinner->setWidget(spinner);  // takes ownership of spinner reference

    this->lbPageNo.reset(gtk_label_new(""), xoj::util::adopt);

    GtkWidget* pageLabel = gtk_label_new(_("Page"));
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_margin_start(pageLabel, 7);
        gtk_widget_set_margin_end(pageLabel, 7);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo.get(), GTK_ALIGN_BASELINE);
        gtk_widget_set_margin_start(this->lbPageNo.get(), 7);
        gtk_widget_set_margin_end(this->lbPageNo.get(), 7);
    } else {
        gtk_widget_set_halign(pageLabel, GTK_ALIGN_CENTER);
        gtk_widget_set_margin_top(pageLabel, 7);
        gtk_widget_set_margin_bottom(pageLabel, 7);
        gtk_widget_set_halign(spinner, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(this->lbPageNo.get(), GTK_ALIGN_CENTER);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo.get(), GTK_ALIGN_BASELINE);
        gtk_label_set_justify(GTK_LABEL(this->lbPageNo.get()), GTK_JUSTIFY_CENTER);
        gtk_widget_set_margin_top(this->lbPageNo.get(), 7);
        gtk_widget_set_margin_bottom(this->lbPageNo.get(), 7);
    }

    this->item.reset(gtk_box_new(orientation, 1), xoj::util::adopt);
    GtkBox* box = GTK_BOX(this->item.get());
    gtk_box_append(box, pageLabel);
    gtk_box_append(box, spinner);
    gtk_box_append(box, this->lbPageNo.get());

    updateLabels();

    return this->item.get();
}
