#include "ToolPageSpinner.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref_sink

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "gui/widgets/SpinPageAdapter.h"          // for SpinPageAdapter
#include "util/i18n.h"                            // for FS, _, _F, C_F

class ActionHandler;

ToolPageSpinner::ToolPageSpinner(ActionHandler* handler, std::string id, ActionType type,
                                 IconNameHelper iconNameHelper):
        AbstractToolItem(std::move(id), handler, type, nullptr), iconNameHelper(iconNameHelper) {
    this->pageSpinner = new SpinPageAdapter();
}

ToolPageSpinner::~ToolPageSpinner() {
    delete this->pageSpinner;
    g_clear_object(&this->lbVerticalPdfPage);
    g_clear_object(&this->lbPageNo);
    g_clear_object(&this->box);
}

auto ToolPageSpinner::getPageSpinner() const -> SpinPageAdapter* { return pageSpinner; }

void ToolPageSpinner::setPageInfo(const size_t pageCount, const size_t pdfPage) {
    this->pageCount = pageCount;
    this->pdfPage = pdfPage;
    if (this->lbPageNo) {
        updateLabels();
    }
}

void ToolPageSpinner::updateLabels() {
    std::string ofString = FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}") % this->pageCount);
    if (this->orientation == GTK_ORIENTATION_HORIZONTAL) {
        std::string pdfString;
        if (this->pdfPage > 0) {  // zero means that theres no pdf currently
            pdfString = std::string(", ") + FS(_F("PDF Page {1}") % this->pdfPage);
        }
        gtk_label_set_text(GTK_LABEL(lbPageNo), (ofString + pdfString).c_str());
    } else {
        gtk_label_set_text(GTK_LABEL(lbPageNo), ofString.c_str());
        if (this->pdfPage > 0) {  // zero means that theres no pdf currently
            gtk_label_set_text(GTK_LABEL(lbVerticalPdfPage), FS(_F("PDF {1}") % this->pdfPage).c_str());
            if (gtk_widget_get_parent(this->lbVerticalPdfPage) == nullptr) {
                // re-add pdf label if it has been removed previously
                gtk_box_pack_start(GTK_BOX(box), this->lbVerticalPdfPage, false, false, 0);
                gtk_widget_show(this->lbVerticalPdfPage);
            }
        } else {
            if (gtk_widget_get_parent(this->lbVerticalPdfPage) != nullptr) {
                gtk_container_remove(GTK_CONTAINER(box), this->lbVerticalPdfPage);
            }
        }
    }
}

auto ToolPageSpinner::getToolDisplayName() const -> std::string { return _("Page number"); }

auto ToolPageSpinner::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconNameHelper.iconName("page-spinner").c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolPageSpinner::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }

auto ToolPageSpinner::newItem() -> GtkToolItem* {
    if (this->pageSpinner->hasWidget()) {
        this->pageSpinner->removeWidget();
    }
    GtkWidget* spinner = gtk_spin_button_new_with_range(0, 1, 1);
    gtk_orientable_set_orientation(reinterpret_cast<GtkOrientable*>(spinner), orientation);
    g_object_ref_sink(spinner);
    this->pageSpinner->setWidget(spinner);  // takes ownership of spinner reference

    if (this->lbPageNo) {
        g_object_unref(this->lbPageNo);
    }
    this->lbPageNo = gtk_label_new("");
    g_object_ref_sink(this->lbPageNo);

    if (this->lbVerticalPdfPage) {
        g_clear_object(&this->lbVerticalPdfPage);
    }

    GtkWidget* pageLabel = gtk_label_new(_("Page"));
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo, GTK_ALIGN_BASELINE);
    } else {
        this->lbVerticalPdfPage = gtk_label_new("");
        g_object_ref_sink(this->lbVerticalPdfPage);

        gtk_widget_set_halign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(spinner, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(this->lbPageNo, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(lbVerticalPdfPage, GTK_ALIGN_BASELINE);
    }

    if (this->box) {
        g_object_unref(this->box);
    }
    this->box = gtk_box_new(orientation, 1);
    g_object_ref_sink(this->box);
    gtk_box_pack_start(GTK_BOX(box), pageLabel, false, false, 7);
    gtk_box_pack_start(GTK_BOX(box), spinner, false, false, 0);
    gtk_box_pack_start(GTK_BOX(box), this->lbPageNo, false, false, 7);

    GtkToolItem* it = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(it), box);

    updateLabels();

    return it;
}

auto ToolPageSpinner::createItem(bool horizontal) -> GtkToolItem* {
    this->orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;

    this->item = createTmpItem(horizontal);

    return this->item;
}

auto ToolPageSpinner::createTmpItem(bool horizontal) -> GtkToolItem* {
    GtkToolItem* item = AbstractToolItem::createTmpItem(horizontal);
    g_object_ref(item);
    return item;
}
