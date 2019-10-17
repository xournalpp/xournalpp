#include "XojOpenDlg.h"

#include <config.h>
#include <i18n.h>
#include <StringUtils.h>
#include <XojPreviewExtractor.h>

#include <gio/gio.h>

XojOpenDlg::XojOpenDlg(GtkWindow* win, Settings* settings)
 : win(win),
   settings(settings)
{
	dialog = gtk_file_chooser_dialog_new(_("Open file"), win, GTK_FILE_CHOOSER_ACTION_OPEN,
										 _("_Cancel"), GTK_RESPONSE_CANCEL,
										 _("_Open"), GTK_RESPONSE_OK, nullptr);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);

	const gchar* currentFolder = nullptr;
	if (!settings->getLastOpenPath().isEmpty())
	{
		currentFolder = settings->getLastOpenPath().c_str();
	}
	else
	{
		g_warning("lastOpenPath is not set!");
		currentFolder = g_get_home_dir();
	}
	gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(dialog), currentFolder);
}

XojOpenDlg::~XojOpenDlg()
{
	if (dialog)
	{
		gtk_widget_destroy(dialog);
	}
	dialog = nullptr;
}

void XojOpenDlg::addFilterAllFiles()
{
	GtkFileFilter* filterAll = gtk_file_filter_new();
	gtk_file_filter_set_name(filterAll, _("All files"));
	gtk_file_filter_add_pattern(filterAll, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterAll);
}

void XojOpenDlg::addFilterPdf()
{
	GtkFileFilter* filterPdf = gtk_file_filter_new();
	gtk_file_filter_set_name(filterPdf, _("PDF files"));
	gtk_file_filter_add_pattern(filterPdf, "*.pdf");
	gtk_file_filter_add_pattern(filterPdf, "*.PDF");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterPdf);
}

void XojOpenDlg::addFilterXoj()
{
	GtkFileFilter* filterXoj = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXoj, _("Xournal files"));
	gtk_file_filter_add_pattern(filterXoj, "*.xoj");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXoj);
}

void XojOpenDlg::addFilterXopp()
{
	GtkFileFilter* filterXopp = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXopp, _("Xournal++ files"));
	gtk_file_filter_add_pattern(filterXopp, "*.xopp");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopp);
}

void XojOpenDlg::addFilterXopt()
{
	GtkFileFilter* filterXopt = gtk_file_filter_new();
	gtk_file_filter_set_name(filterXopt, _("Xournal++ template"));
	gtk_file_filter_add_pattern(filterXopt, "*.xopt");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterXopt);
}

Path XojOpenDlg::runDialog()
{
	gtk_window_set_transient_for(GTK_WINDOW(dialog), win);
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
	{
		gtk_widget_destroy(dialog);
		dialog = nullptr;
		return Path("");
	}

	Path file(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
	settings->setLastOpenPath(file.getParentPath().str());

	return file;
}

Path XojOpenDlg::showOpenTemplateDialog()
{
	addFilterAllFiles();
	addFilterXopt();

	return runDialog();
}

Path XojOpenDlg::showOpenDialog(bool pdf, bool& attachPdf)
{
	if (!pdf)
	{
		GtkFileFilter* filterSupported = gtk_file_filter_new();
		gtk_file_filter_set_name(filterSupported, _("Supported files"));
		gtk_file_filter_add_pattern(filterSupported, "*.xoj");
		gtk_file_filter_add_pattern(filterSupported, "*.xopp");
		gtk_file_filter_add_pattern(filterSupported, "*.xopt");
		gtk_file_filter_add_pattern(filterSupported, "*.pdf");
		gtk_file_filter_add_pattern(filterSupported, "*.PDF");
		gtk_file_filter_add_pattern(filterSupported, "*.moj"); // MrWriter
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filterSupported);

		addFilterXoj();
		addFilterXopt();
		addFilterXopp();
	}

	addFilterPdf();
	addFilterAllFiles();

	GtkWidget* attachOpt = nullptr;
	if (pdf)
	{
		attachOpt = gtk_check_button_new_with_label(_("Attach file to the journal"));
		g_object_ref(attachOpt);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(attachOpt), false);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog), attachOpt);
	}

	GtkWidget* image = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), image);
	g_signal_connect(dialog, "update-preview", G_CALLBACK(updatePreviewCallback), nullptr);

	auto lastOpenPath = this->settings->getLastOpenPath();
	if (!lastOpenPath.isEmpty())
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(this->dialog), lastOpenPath.c_str());
	}

	auto lastSavePath = this->settings->getLastSavePath();
	if (!lastSavePath.isEmpty())
	{
		gtk_file_chooser_add_shortcut_folder(GTK_FILE_CHOOSER(this->dialog), lastSavePath.c_str(), nullptr);
	}

	Path file = runDialog();

	if (attachOpt)
	{
		attachPdf = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(attachOpt));
		g_object_unref(attachOpt);
	}

	if (!file.isEmpty())
	{
		g_message("lastOpenPath set");
		this->settings->setLastOpenPath(file.getParentPath().str());
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

	Path filepath = filename;
	g_free(filename);
	filename = nullptr;

	if (!filepath.hasXournalFileExt())
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

	GError* error = nullptr;
	gsize dataLen = 0;
	unsigned char* imageData = extractor.getData(dataLen);

	GInputStream* in = g_memory_input_stream_new_from_data(imageData, dataLen, nullptr);
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_stream(in, nullptr, &error);
	if (error != nullptr)
	{
		g_warning("Could not load preview image, error: %s\n", error->message);
		g_error_free(error);
	}

	g_input_stream_close(in, nullptr, nullptr);

	if (pixbuf)
	{
		GtkWidget * image = gtk_file_chooser_get_preview_widget(fileChooser);
		gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
		g_object_unref(pixbuf);
		gtk_file_chooser_set_preview_widget_active(fileChooser, true);
	}
}
