#include "XojOpenDlg.h"

#include <config.h>
#include "util/XojPreviewExtractor.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>

path XojOpenDlg::showOpenDialog(GtkWindow* win, Settings* settings, bool pdf, bool& attachPdf)
{

	GtkWidget* dialog = gtk_file_chooser_dialog_new(_("Open file"), win,
													GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
													GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	GtkFileFilter* filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");

	GtkFileFilter* filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");

	GtkFileFilter* filterSupported = gtk_file_filter_new();
	gtk_file_filter_set_name(filterSupported, _("Supported files"));
	gtk_file_filter_add_pattern(filterSupported, "*.xoj");
	gtk_file_filter_add_pattern(filterSupported, "*.pdf");
	gtk_file_filter_add_pattern(filterSupported, "*.PDF");

	if (!pdf)
	{
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);

	if (!settings->getLastSavePath().empty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}
	else
	{
		g_warning("lastSavePath is not set!", 0);
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
	}

	GtkWidget* attachOpt = NULL;
	if (pdf)
	{
		attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), FALSE);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
	}

	GtkWidget* image = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), NULL);


	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return path("");
	}
	path file(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

	if (attachOpt)
	{
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
	}
	
	settings->setLastSavePath(file.parent_path());

	gtk_widget_destroy(dialog);

	return file;
}

void XojOpenDlg::updatePreviewCallback(GtkFileChooser* fileChooser, void* userData)
{
	gchar* filename = gtk_file_chooser_get_preview_filename(fileChooser);

	if (!filename)
	{
		gtk_file_chooser_set_preview_widget_active(fileChooser, false);
		return;
	}

	string filepath = filename;
	g_free(filename);
	filename = NULL;

	if (filepath.size() <= 4 || filepath.substr(filepath.size() - 4) != ".xoj")
	{
		gtk_file_chooser_set_preview_widget_active(fileChooser, false);
		return;
	}

	XojPreviewExtractor extractor;
	PreviewExtractResult result = extractor.readFile(filepath);

	if (result != PREVIEW_RESULT_IMAGE_READ)
	{
		gtk_file_chooser_set_preview_widget_active(fileChooser, false);
		return;
	}

	GError* error = NULL;
	GInputStream* in = g_memory_input_stream_new_from_data(extractor.getData().c_str(), extractor.getData().length(), NULL);
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(in, NULL, &error);
	g_input_stream_close(in, NULL, &error);

	if (pixbuf)
	{
		GtkWidget * image = gtk_file_chooser_get_preview_widget(fileChooser);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
		gtk_file_chooser_set_preview_widget_active(fileChooser, true);
	}
}
