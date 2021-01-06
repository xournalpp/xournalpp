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

void ToolPageSpinner::setText(const string& text) {
    if (lbPageNo) {
        gtk_label_set_text(GTK_LABEL(lbPageNo), text.c_str());
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
    this->lbPageNo = gtk_label_new(this->lbPageNo ? gtk_label_get_text(GTK_LABEL(lbPageNo)) : "");

    if (horizontal) {
        gtk_widget_set_valign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_valign(this->lbPageNo, GTK_ALIGN_BASELINE);
    } else {
        gtk_widget_set_halign(pageLabel, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(spinner, GTK_ALIGN_BASELINE);
        gtk_widget_set_halign(this->lbPageNo, GTK_ALIGN_BASELINE);
    }

    GtkWidget* box = gtk_box_new(orientation, 1);
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