#include "ImageOpenDlg.h"
#include "../settings/Settings.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ImageOpenDlg::ImageOpenDlg() {
}

ImageOpenDlg::~ImageOpenDlg() {
}

GFile * ImageOpenDlg::show(GtkWindow * win, Settings * settings, bool localOnly, bool * attach) {
	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Open Image"), win, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
			GTK_RESPONSE_OK, NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), localOnly);

	GtkFileFilter * filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Images"));
	gtk_file_filter_add_pixbuf_formats(filterSupported);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

	if (!settings->getLastImagePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastImagePath().c_str());
	}

	GtkWidget * cbAttach;
	if (attach) {
		cbAttach = gtk_check_button_new_with_label(_("Attach file to the journal"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cbAttach), false);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), cbAttach);
	}

	GtkWidget * image = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), NULL);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return NULL;
	}
	GFile * file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
	if (attach) {
		*attach = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbAttach));
	}

	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastImagePath(folder);
	g_free(folder);

	gtk_widget_destroy(dialog);

	return file;
}

// Source: Empathy

GdkPixbuf * ImageOpenDlg::pixbufScaleDownIfNecessary(GdkPixbuf * pixbuf, gint maxSize) {
	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);

	if (width > maxSize || height > maxSize) {
		double factor = (gdouble) maxSize / MAX(width, height);

		width = width * factor;
		height = height * factor;

		return gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_HYPER);
	}

	return (GdkPixbuf *)g_object_ref(pixbuf);
}

void ImageOpenDlg::updatePreviewCallback(GtkFileChooser * fileChooser, void * userData) {
	gchar * filename = gtk_file_chooser_get_preview_filename(fileChooser);

	if (filename) {
		GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
		GtkWidget * image = gtk_file_chooser_get_preview_widget(fileChooser);

		if (pixbuf) {
			GdkPixbuf * scaled_pixbuf = pixbufScaleDownIfNecessary(pixbuf, 256);
			gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);
			g_object_unref(scaled_pixbuf);
			g_object_unref(pixbuf);
		} else {
			gtk_image_set_from_stock(GTK_IMAGE(image), "gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
		}

		g_free(filename);
	}

	gtk_file_chooser_set_preview_widget_active(fileChooser, true);
}

