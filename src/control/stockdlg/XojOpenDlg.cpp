#include "XojOpenDlg.h"

#include <config.h>
#include <i18n.h>
#include <XojPreviewExtractor.h>

#include <gio/gio.h>


XojOpenDlg::XojOpenDlg(GtkWindow* win, Settings* settings)
 : win(win),
   settings(settings)
{
	XOJ_INIT_TYPE(XojOpenDlg);

	dialog = gtk_file_chooser_dialog_new(_C("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN,
										 _C("_Cancel"), GTK_RESPONSE_CANCEL,
										 _C("_Open"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	if (!settings->getLastSavePath().empty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), settings->getLastSavePath().c_str());
	}
	else
	{
		g_warning("lastSavePath is not set!");
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), g_get_home_dir());
	}
}

XojOpenDlg::~XojOpenDlg()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	gtk_widget_destroy(dialog);
	dialog = NULL;

	XOJ_RELEASE_TYPE(XojOpenDlg);
}

void XojOpenDlg::addFilterAllFiles()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _C("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
}

void XojOpenDlg::addFilterPdf()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _C("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
}

void XojOpenDlg::addFilterXoj()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _C("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
}

void XojOpenDlg::addFilterXojt()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterXojt = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXojt, _C("Xournal++ template"));
	gtk_file_filter_add_pattern(filterXojt, "*.xojt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXojt);
}

path XojOpenDlg::runDialog()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		return path("");
	}

	path file(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
	settings->setLastSavePath(file.parent_path());

	return file;
}

path XojOpenDlg::showOpenTemplateDialog()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	addFilterAllFiles();
	addFilterXojt();

	return runDialog();
}

path XojOpenDlg::showOpenDialog(bool pdf, bool& attachPdf)
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	if (!pdf)
	{
		GtkFileFilter* filterSupported = gtk_file_filter_new();
		gtk_file_filter_set_name(filterSupported, _C("Supported files"));
		gtk_file_filter_add_pattern(filterSupported, "*.xoj");

		// TODO: Implement template loading from file open
		// gtk_file_filter_add_pattern(filterSupported, "*.xojt");
		gtk_file_filter_add_pattern(filterSupported, "*.pdf");
		gtk_file_filter_add_pattern(filterSupported, "*.PDF");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

		addFilterXoj();

		// TODO: Implement template loading from file open
		// addFilterXojt();
	}

	addFilterPdf();
	addFilterAllFiles();

	GtkWidget* attachOpt = NULL;
	if (pdf)
	{
		attachOpt = gtk_check_button_new_with_label(_C("Attach file to the journal"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), FALSE);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
	}

	GtkWidget* image = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), NULL);

	path file = runDialog();

	if (attachOpt)
	{
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
	}

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
