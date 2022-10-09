#include "ToolbarDragDropHelper.h"

#include <cairo.h>        // for cairo_surface_set_device_offset, cairo_surf...
#include <gio/gio.h>      // for GIcon
#include <glib-object.h>  // for g_object_get, G_OBJECT
#include <glib.h>         // for g_warning, gchar

namespace ToolbarDragDropHelper {

const GdkAtom atomToolItem = gdk_atom_intern_static_string("application/xournal-ToolbarItem");
const GtkTargetEntry dropTargetEntry = {const_cast<char*>("move-buffer"), GTK_TARGET_SAME_APP, 1};

auto gdk_context_set_icon_from_image(GdkDragContext* ctx, GtkWidget* widget) -> bool {
    auto image = GTK_IMAGE(widget);
    auto storage = gtk_image_get_storage_type(image);
    switch (storage) {
        case GTK_IMAGE_PIXBUF: {
            gtk_drag_set_icon_pixbuf(ctx, gtk_image_get_pixbuf(image), -2, -2);
            return true;
        }
        case GTK_IMAGE_ICON_NAME: {
            gchar const* icon_name{};
            gtk_image_get_icon_name(image, &icon_name, nullptr);
            gtk_drag_set_icon_name(ctx, icon_name, -2, -2);
            return true;
        }
        case GTK_IMAGE_GICON: {
#ifndef __APPLE__
            GIcon* icon{};
            gtk_image_get_gicon(image, &icon, nullptr);
            gtk_drag_set_icon_gicon(ctx, icon, -2, -2);
            return true;
#else
            // Todo (fabian): check, if `gtk_drag_set_icon_gicon` is still unimplemented
            g_warning("ToolbarDragDropHelper::gdk_context_set_icon_from_image: GTK_IMAGE_GICON is not handled because "
                      "`gtk_drag_set_icon_gicon` is not implemented. Please create an issue, if this has changed.");
            break;
#endif
        }
        case GTK_IMAGE_SURFACE: {
            cairo_surface_t* surface{};
            g_object_get(G_OBJECT(image), "surface", &surface, nullptr);
            cairo_surface_set_device_offset(surface, -2, -2);
            gtk_drag_set_icon_surface(ctx, surface);
            return true;
        }
        case GTK_IMAGE_EMPTY: {  ///< Does nothing

            g_warning("ToolbarDragDropHelper::gdk_context_set_icon_from_image: Image storage is empty");
            break;
        }
        case GTK_IMAGE_ICON_SET: {  ///< Deprecated
            g_warning("ToolbarDragDropHelper::gdk_context_set_icon_from_image: Image storage GTK_IMAGE_ICON_SET is "
                      "deprecated");
            break;
        }
        case GTK_IMAGE_STOCK: {  ///< Deprecated
            g_warning("ToolbarDragDropHelper::gdk_context_set_icon_from_image: Image storage GTK_IMAGE_STOCK is "
                      "deprecated");
            break;
        }
        case GTK_IMAGE_ANIMATION: {  ///< Can't be handled as we know
            g_warning("ToolbarDragDropHelper::gdk_context_set_icon_from_image: Image storage GTK_IMAGE_ANIMATION can't "
                      "be handled");
            break;
        }
    }
    gtk_drag_set_icon_widget(ctx, widget, -2, -2);
    return false;
}

void dragDestAddToolbar(GtkWidget* target) {
    GtkTargetList* targetList = gtk_drag_dest_get_target_list(target);
    if (targetList) {
        gtk_target_list_ref(targetList);
    } else {
        targetList = gtk_target_list_new(nullptr, 0);
    }

    // If not exist add, else do nothing
    if (!gtk_target_list_find(targetList, atomToolItem, nullptr)) {
        gtk_target_list_add(targetList, atomToolItem, 0, 0);
    }

    gtk_drag_dest_set_target_list(target, targetList);
    gtk_target_list_unref(targetList);
}

void dragSourceAddToolbar(GtkWidget* widget) {
    GtkTargetList* targetList = gtk_drag_source_get_target_list(widget);
    if (targetList) {
        // List contains already this type
        if (gtk_target_list_find(targetList, atomToolItem, nullptr)) {
            return;
        }

        gtk_target_list_ref(targetList);
    } else {
        targetList = gtk_target_list_new(nullptr, 0);
    }
    gtk_target_list_add(targetList, atomToolItem, 0, 0);
    gtk_drag_source_set_target_list(widget, targetList);
    gtk_target_list_unref(targetList);
}
}  // namespace ToolbarDragDropHelper
