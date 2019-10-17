#include "ToolbarDragDropHelper.h"

GdkAtom ToolbarDragDropHelper::atomToolItem = gdk_atom_intern_static_string("application/xournal-ToolbarItem");
GtkTargetEntry ToolbarDragDropHelper::dropTargetEntry = { (gchar *)"move-buffer", GTK_TARGET_SAME_APP, 1 };

ToolbarDragDropHelper::ToolbarDragDropHelper() { }

ToolbarDragDropHelper::~ToolbarDragDropHelper() { }

/**
 * Get a GDK Pixbuf from GTK widget image
 */
GdkPixbuf* ToolbarDragDropHelper::getImagePixbuf(GtkImage* image)
{
	switch (gtk_image_get_storage_type(image))
	{
	case GTK_IMAGE_PIXBUF:
		return (GdkPixbuf*) g_object_ref(gtk_image_get_pixbuf(image));

	case GTK_IMAGE_ICON_NAME:
	{
		const gchar* iconName = nullptr;
		gtk_image_get_icon_name(image, &iconName, nullptr);
		return gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), iconName, 22, (GtkIconLookupFlags) 0, nullptr);
	}

	default:
		g_warning("Image storage type %d not handled", gtk_image_get_storage_type(image));
		return nullptr;
	}
}

void ToolbarDragDropHelper::dragDestAddToolbar(GtkWidget* target)
{
	GtkTargetList* targetList = gtk_drag_dest_get_target_list(target);
	if (targetList)
	{
		gtk_target_list_ref(targetList);
	}
	else
	{
		targetList = gtk_target_list_new(nullptr, 0);
	}

	// If not exist add, else do nothing
	if (!gtk_target_list_find(targetList, atomToolItem, nullptr))
	{
		gtk_target_list_add(targetList, atomToolItem, 0, 0);
	}

	gtk_drag_dest_set_target_list(target, targetList);
	gtk_target_list_unref(targetList);
}

void ToolbarDragDropHelper::dragSourceAddToolbar(GtkWidget* widget)
{
	GtkTargetList* targetList = gtk_drag_source_get_target_list(widget);
	if (targetList)
	{
		// List contains already this type
		if (gtk_target_list_find(targetList, atomToolItem, nullptr))
		{
			return;
		}

		gtk_target_list_ref(targetList);
	}
	else
	{
		targetList = gtk_target_list_new(nullptr, 0);
	}
	gtk_target_list_add(targetList, atomToolItem, 0, 0);
	gtk_drag_source_set_target_list(widget, targetList);
	gtk_target_list_unref(targetList);
}
