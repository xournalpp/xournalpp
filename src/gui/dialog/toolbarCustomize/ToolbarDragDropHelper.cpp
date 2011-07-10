#include "ToolbarDragDropHelper.h"

GdkAtom ToolbarDragDropHelper::atomToolItem = gdk_atom_intern_static_string("application/xournal-ToolbarItem");

ToolbarDragDropHelper::ToolbarDragDropHelper() {
}

ToolbarDragDropHelper::~ToolbarDragDropHelper() {
}

void ToolbarDragDropHelper::dragDestAddToolbar(GtkWidget * target) {
	GtkTargetList * targetList = gtk_drag_dest_get_target_list(target);
	if (targetList) {
		gtk_target_list_ref(targetList);
	} else {
		targetList = gtk_target_list_new(NULL, 0);
	}

	// If not exist add, else do nothing
	if (!gtk_target_list_find(targetList, atomToolItem, NULL)) {
		gtk_target_list_add(targetList, atomToolItem, 0, NULL);
	}

	gtk_drag_dest_set_target_list(target, targetList);
	gtk_target_list_unref(targetList);
}

void ToolbarDragDropHelper::dragSourceAddToolbar(GtkWidget * widget) {
	GtkTargetList * targetList = gtk_drag_source_get_target_list(widget);
	if (targetList) {
		gtk_target_list_ref(targetList);
	} else {
		targetList = gtk_target_list_new(NULL, 0);
	}
	gtk_target_list_add(targetList, atomToolItem, 0, NULL);
	gtk_drag_source_set_target_list(widget, targetList);
	gtk_target_list_unref(targetList);
}
