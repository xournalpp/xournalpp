#include "ImageOpenDlg.h"

#include <algorithm>  // for max
#include <string>     // for string

#include <glib-object.h>  // for g_object_unref, g_object_ref

#include "control/settings/Settings.h"  // for Settings
#include "util/PathUtil.h"              // for fromGFilename, toGFilename
#include "util/i18n.h"                  // for _

#include "filesystem.h"  // for path

auto ImageOpenDlg::show(GtkWindow* win, Settings* settings, bool localOnly, bool* attach) -> GFile* {
    GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Open Image"), win, GTK_FILE_CHOOSER_ACTION_OPEN, _("_Cancel"),
                                                    GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_OK, nullptr);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), localOnly);

    GtkFileFilter* filterSupported = gtk_file_filter_new();
    gtk_file_filter_set_name(filterSupported, _("Images"));
    gtk_file_filter_add_pixbuf_formats(filterSupported);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

    if (!settings->getLastImagePath().empty()) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                            Util::toGFilename(settings->getLastImagePath()).c_str());
    }

    GtkWidget* cbAttach = nullptr;
    if (attach) {
        cbAttach = gtk_check_button_new_with_label(_("Attach file to the journal"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbAttach), false);
        gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), cbAttach);
    }

    GtkWidget* image = gtk_image_new();
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
    g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), nullptr);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
        gtk_widget_destroy(dialog);
        return nullptr;
    }
    GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
    if (attach) {
        *attach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAttach));
    }


    // e.g. from last used files, there is no folder selected
    // in this case do not store the folder
    if (auto folder = Util::fromGFilename(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog)));
        !folder.empty()) {
        settings->setLastImagePath(folder);
    }
    gtk_widget_destroy(dialog);
    return file;
}

// Source: Empathy

auto ImageOpenDlg::pixbufScaleDownIfNecessary(GdkPixbuf* pixbuf, gint maxSize) -> GdkPixbuf* {
    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);

    if (width > maxSize || height > maxSize) {
        double factor = static_cast<gdouble>(maxSize) / std::max(width, height);

        width = width * factor;
        height = height * factor;

        return gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_HYPER);
    }

    return static_cast<GdkPixbuf*>(g_object_ref(pixbuf));
}

void ImageOpenDlg::updatePreviewCallback(GtkFileChooser* fileChooser, void* userData) {
    gchar* filename = gtk_file_chooser_get_preview_filename(fileChooser);

    if (filename) {
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, nullptr);
        GtkWidget* image = gtk_file_chooser_get_preview_widget(fileChooser);

        if (pixbuf) {
            GdkPixbuf* tmp = gdk_pixbuf_apply_embedded_orientation(pixbuf);
            g_set_object(&pixbuf, tmp);
            GdkPixbuf* scaled_pixbuf = pixbufScaleDownIfNecessary(pixbuf, 256);
            gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);
            g_object_unref(scaled_pixbuf);
            g_object_unref(tmp);
            g_object_unref(pixbuf);
        } else {
            gtk_image_set_from_icon_name(GTK_IMAGE(image), "dialog-question", GTK_ICON_SIZE_DIALOG);
        }

        g_free(filename);
    }

    gtk_file_chooser_set_preview_widget_active(fileChooser, true);
}
