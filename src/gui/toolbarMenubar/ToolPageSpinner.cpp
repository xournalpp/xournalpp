#include "ToolPageSpinner.h"

#include <utility>

#include <config.h>

#include "gui/GladeGui.h"
#include "gui/widgets/SpinPageAdapter.h"

#include "i18n.h"

ToolPageSpinner::ToolPageSpinner(GladeGui* gui, ActionHandler* handler, string id, ActionType type):
        AbstractToolItem(std::move(id), handler, type, nullptr) {
    this->gui = gui;
    this->pageSpinner = new SpinPageAdapter();
}

ToolPageSpinner::~ToolPageSpinner() {
    delete this->pageSpinner;
    if (this->lbVerticalPdfPage) {
        g_object_unref(this->lbVerticalPdfPage);
    }
    g_object_unref(this->lbPageNo);
    g_object_unref(this->box);
}

auto ToolPageSpinner::getPageSpinner() -> SpinPageAdapter* { return pageSpinner; }

void ToolPageSpinner::setPageInfo(const size_t pagecount, const size_t pdfpage) {
    this->pagecount = pagecount;
    this->pdfpage = pdfpage;
    if (this->lbPageNo) {
        updateLabels();
    }
}

void ToolPageSpinner::updateLabels() {
    string ofString = FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}") % this->pagecount);
    if (this->orientation == GTK_ORIENTATION_HORIZONTAL) {
        string pdfString;
        if (this->pdfpage > 0) {  // zero means that theres no pdf currently
            pdfString = string(", ") + FS(_F("PDF Page {1}") % this->pdfpage);
        }
        gtk_label_set_text(GTK_LABEL(lbPageNo), (ofString + pdfString).c_str());
    } else {
        gtk_label_set_text(GTK_LABEL(lbPageNo), ofString.c_str());
        if (this->pdfpage > 0) {  // zero means that theres no pdf currently
            gtk_label_set_text(GTK_LABEL(lbVerticalPdfPage), FS(_F("PDF {1}") % this->pdfpage).c_str());
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

auto ToolPageSpinner::getToolDisplayName() -> string { return _("Page number"); }

auto ToolPageSpinner::getNewToolIcon() -> GtkWidget* {
    return gtk_image_new_from_icon_name("pageSpinner", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto ToolPageSpinner::newItem() -> GtkToolItem* {
    if (this->pageSpinner->hasWidget()) {
        this->pageSpinner->removeWidget();
    }
    GtkWidget* spinner = gtk_spin_button_new_with_range(0, 1, 1);
    gtk_orientable_set_orientation(reinterpret_cast<GtkOrientable*>(spinner), orientation);

    GtkWidget* pageLabel = gtk_label_new(_("Page"));

    if (this->lbPageNo) {
        g_object_unref(this->lbPageNo);
    }
    this->lbPageNo = gtk_label_new("");

    if (this->lbVerticalPdfPage) {
        g_object_unref(this->lbVerticalPdfPage);
    }
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        this->lbVerticalPdfPage = nullptr;
        gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo, GTK_ALIGN_BASELINE);
    } else {
        this->lbVerticalPdfPage = gtk_label_new("");
        gtk_widget_set_halign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(spinner, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(this->lbPageNo, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(lbVerticalPdfPage, GTK_ALIGN_BASELINE);
    }
    this->pageSpinner->setWidget(spinner);  // takes ownership of spinner reference

    if (this->box) {
        g_object_unref(box);
    }
    box = gtk_box_new(orientation, 1);
    gtk_box_pack_start(GTK_BOX(box), pageLabel, false, false, 7);
    g_object_unref(pageLabel);
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
