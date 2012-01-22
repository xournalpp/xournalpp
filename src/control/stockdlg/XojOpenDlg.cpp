#include "XojOpenDlg.h"

#include <config.h>
#include <glib/gi18n-lib.h>

String XojOpenDlg::showOpenDialog(GtkWindow * win, Settings * settings, bool pdf, bool & attachPdf) {

	GtkWidget * dialog = gtk_file_chooser_dialog_new(_("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter *filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");

	GtkFileFilter *filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");

	GtkFileFilter *filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");

	GtkFileFilter *filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Supported files"));
	gtk_file_filter_add_pattern(filterSupported, "*.xoj");
	gtk_file_filter_add_pattern(filterSupported, "*.pdf");
	gtk_file_filter_add_pattern(filterSupported, "*.PDF");

	if (!pdf) {
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);

	if (!settings->getLastSavePath().isEmpty()) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}

	GtkWidget * attachOpt = NULL;
	if (pdf) {
		attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), FALSE);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
	}

	GtkWidget * image = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), NULL);


	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
		gtk_widget_destroy(dialog);
		return NULL;
	}
	char * name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	if (attachOpt) {
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
	}

	String filename = name;
	char * folder = gtk_file_chooser_get_current_folder_uri(GTK_FILE_CHOOSER(dialog));
	settings->setLastSavePath(folder);
	g_free(folder);
	g_free(name);

	gtk_widget_destroy(dialog);

	return filename;
}

void XojOpenDlg::updatePreviewCallback(GtkFileChooser * fileChooser, void * userData) {
	// TODO LOW PRIO create preview if the file contains one
//	gchar * filename = gtk_file_chooser_get_preview_filename(fileChooser);
//
//	if (filename) {
//		GdkPixbuf * pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
//		GtkWidget * image = gtk_file_chooser_get_preview_widget(fileChooser);
//
//		if (pixbuf) {
//			GdkPixbuf * scaled_pixbuf = pixbufScaleDownIfNecessary(pixbuf, 256);
//			gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);
//			g_object_unref(scaled_pixbuf);
//			g_object_unref(pixbuf);
//		} else {
//			gtk_image_set_from_stock(GTK_IMAGE(image), "gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
//		}
//
//		g_free(filename);
//	}
//
//	gtk_file_chooser_set_preview_widget_active(fileChooser, true);
}

