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
    this->pageSpinner = nullptr;
}

auto ToolPageSpinner::getPageSpinner() -> SpinPageAdapter* { return pageSpinner; }

void ToolPageSpinner::setPageInfo(const size_t pagecount, const size_t pdfpage) {
    this->pagecount = pagecount;
    this->pdfpage = pdfpage;
    if (lbPageNo) {
        updateLabels();
    }
}

void ToolPageSpinner::updateLabels() {
    string ofString = FS(C_F("Page {pagenumber} \"of {pagecount}\"", " of {1}") % this->pagecount);
    if (this->horizontal) {
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
    GtkOrientation orientation = horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;

    GtkWidget* spinner = this->pageSpinner->newWidget();
    gtk_orientable_set_orientation(reinterpret_cast<GtkOrientable*>(spinner), orientation);

    GtkWidget* pageLabel = gtk_label_new(_("Page"));
    this->lbPageNo = gtk_label_new("");

    if (horizontal) {
        this->lbVerticalPdfPage = nullptr;
        gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo, GTK_ALIGN_BASELINE);
    } else {
        this->lbVerticalPdfPage = gtk_label_new("");
        gtk_widget_set_halign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(this->lbPageNo, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(lbVerticalPdfPage, GTK_ALIGN_BASELINE);
    }

    updateLabels();

    box = gtk_box_new(orientation, 1);
    gtk_box_pack_start(GTK_BOX(box), pageLabel, false, false, 7);
    gtk_box_pack_start(GTK_BOX(box), spinner, false, false, 0);
    gtk_box_pack_start(GTK_BOX(box), this->lbPageNo, false, false, 7);

    GtkToolItem* it = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(it), box);

    return it;
}

auto ToolPageSpinner::createItem(bool horizontal) -> GtkToolItem* {
    this->horizontal = horizontal;

    this->item = createTmpItem(horizontal);
    g_object_ref(this->item);

    if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
        g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
    }

    return this->item;
}

auto ToolPageSpinner::createTmpItem(bool horizontal) -> GtkToolItem* {
    GtkToolItem* item = AbstractToolItem::createTmpItem(horizontal);
    g_object_ref(item);
    return item;
}