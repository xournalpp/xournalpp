#include "ToolPageSpinner.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ToolPageSpinner::ToolPageSpinner(ActionHandler * handler, String id, ActionType type) :
	AbstractToolItem(id, handler, type, NULL) {
	pageSpinner = gtk_spin_button_new_with_range(0, 0, 1);
	lbPageNo = NULL;
}

ToolPageSpinner::~ToolPageSpinner() {
}

GtkWidget * ToolPageSpinner::getPageSpinner() {
	return pageSpinner;
}

void ToolPageSpinner::setText(String text) {
	if (lbPageNo) {
		gtk_label_set_text(GTK_LABEL(lbPageNo), text.c_str());
	}
}

GtkToolItem * ToolPageSpinner::newItem() {
	GtkToolItem * it = gtk_tool_item_new();

	GtkWidget * hbox = gtk_hbox_new(false, 1);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Page")), false, false, 7);

	gtk_box_pack_start(GTK_BOX(hbox), pageSpinner, false, false, 0);

	lbPageNo = gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(hbox), lbPageNo, false, false, 0);

	gtk_container_add(GTK_CONTAINER(it), hbox);

	return it;
}

