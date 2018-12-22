#include "XojOpenDlg.h"

#include <config.h>
#include <i18n.h>
#include <XojPreviewExtractor.h>
#include <Util.h>

#include <gio/gio.h>
#include <boost/algorithm/string.hpp>


XojOpenDlg::XojOpenDlg(GtkWindow* win, Settings* settings)
 : win(win),
   settings(settings)
{
	XOJ_INIT_TYPE(XojOpenDlg);

	dialog = gtk_file_chooser_dialog_new(_("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN,
										 _("_Cancel"), GTK_RESPONSE_CANCEL,
										 _("_Open"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	if (!settings->getLastSavePath().empty())
	{
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), PATH_TO_CSTR(settings->getLastSavePath()));
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
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
}

void XojOpenDlg::addFilterPdf()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
}

void XojOpenDlg::addFilterXoj()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
}

void XojOpenDlg::addFilterXopp()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterXopp = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXopp, _("Xournal++ files"));
	gtk_file_filter_add_pattern(filterXopp, "*.xopp");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopp);
}

void XojOpenDlg::addFilterXopt()
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	GtkFileFilter* filterXopt = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXopt, _("Xournal++ template"));
	gtk_file_filter_add_pattern(filterXopt, "*.xopt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopt);
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
	addFilterXopt();

	return runDialog();
}

path XojOpenDlg::showOpenDialog(bool pdf, bool& attachPdf)
{
	XOJ_CHECK_TYPE(XojOpenDlg);

	if (!pdf)
	{
		GtkFileFilter* filterSupported = gtk_file_filter_new();
		gtk_file_filter_set_name(filterSupported, _("Supported files"));
		gtk_file_filter_add_pattern(filterSupported, "*.xoj");
		gtk_file_filter_add_pattern(filterSupported, "*.xopp");
		gtk_file_filter_add_pattern(filterSupported, "*.xopt");
		gtk_file_filter_add_pattern(filterSupported, "*.pdf");
		gtk_file_filter_add_pattern(filterSupported, "*.PDF");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

		addFilterXoj();
		addFilterXopt();
		addFilterXopp();
	}

	addFilterPdf();
	addFilterAllFiles();

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

	string ext = "";
	size_t dotPos = filepath.find_last_of(".");
	if (dotPos != string::npos)
	{
		ext = filepath.substr(dotPos);
		boost::algorithm::to_lower(ext);
	}

	if (!(ext == ".xoj" || ext == ".xopp"))
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
	gsize dataLen = 0;
	unsigned char* imageData = extractor.getData(dataLen);

	GInputStream* in = g_memory_input_stream_new_from_data(imageData, dataLen, NULL);
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(in, NULL, &error);
	if (error != NULL)
	{
		g_warning("Could not load preview image, error: %s\n", error->message);
		g_error_free(error);
	}

	g_input_stream_close(in, NULL, NULL);

	if (pixbuf)
	{
		GtkWidget * image = gtk_file_chooser_get_preview_widget(fileChooser);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
		gtk_file_chooser_set_preview_widget_active(fileChooser, true);
	}
}
