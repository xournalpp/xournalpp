#include "ExportDialog.h"
#include <PageRange.h>

#include <config.h>
#include <glib/gi18n-lib.h>

ExportDialog::ExportDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
                           int pageCount, int currentPage) :
	GladeGui(gladeSearchPath, "export.glade", "exportDialog")
{

	XOJ_INIT_TYPE(ExportDialog);

	this->range = NULL;
	this->pageCount = pageCount;
	this->currentPage = currentPage;
	this->resolution = 72;
	this->settings = settings;
	this->type = EXPORT_FORMAT_PNG;

	GtkFileChooser* chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	gtk_file_chooser_set_current_folder(chooser,
	                                    settings->getLastSavePath().c_str());
	GtkEntry* folderFollower = GTK_ENTRY(get("fcOutText"));
	g_signal_connect(get("fcOutput"), "selection-changed",
	                 G_CALLBACK(&exportSelectionChanged), folderFollower);
}

ExportDialog::~ExportDialog()
{
	XOJ_RELEASE_TYPE(ExportDialog);
}

GList* ExportDialog::getRange()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->range;
}

void ExportDialog::exportSelectionChanged(GtkFileChooser* chooser,
                                          GtkEntry* newFolder)
{
	char* folder = gtk_file_chooser_get_current_folder(chooser);
	gtk_entry_set_text(newFolder, folder);
}

void ExportDialog::handleData()
{
	XOJ_CHECK_TYPE(ExportDialog);

	//	GtkWidget * rdRangeAll = get("rdRangeAll");
	GtkWidget* rdRangeCurrent = get("rdRangeCurrent");
	GtkWidget* rdRangePages = get("rdRangePages");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangePages)))
	{
		this->range = PageRange::parse(gtk_entry_get_text(GTK_ENTRY(get("txtPages"))));
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdRangeCurrent)))
	{
		this->range = g_list_append(this->range, new PageRangeEntry(this->currentPage,
		                                                            this->currentPage));
	}
	else
	{
		this->range = g_list_append(this->range, new PageRangeEntry(0,
		                                                            this->pageCount - 1));
	}

	this->resolution = gtk_spin_button_get_value(GTK_SPIN_BUTTON(
	                                                 get("spPngResolution")));

	GtkWidget* rdFormatPDF = get("rdFormatPDF");
	GtkWidget* rdFormatEPS = get("rdFormatEPS");
	GtkWidget* rdFormatSVG = get("rdFormatSVG");
	//	GtkWidget * rdFormatPNG = get("rdFormatPNG");

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatPDF)))
	{
		this->type = EXPORT_FORMAT_PDF;
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatEPS)))
	{
		this->type = EXPORT_FORMAT_EPS;
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatSVG)))
	{
		this->type = EXPORT_FORMAT_SVG;
	}
	else
	{
		this->type = EXPORT_FORMAT_PNG;
	}

	GtkFileChooser* chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	char* folder = gtk_file_chooser_get_current_folder(chooser);
	this->settings->setLastSavePath(folder);
	g_free(folder);
}

ExportFormtType ExportDialog::getFormatType()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->type;
}

int ExportDialog::getPngDpi()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return this->resolution;
}

String ExportDialog::getFolder()
{
	XOJ_CHECK_TYPE(ExportDialog);

	GtkFileChooser* chooser = GTK_FILE_CHOOSER(get("fcOutput"));
	char* folder = gtk_file_chooser_get_current_folder(chooser);
	String f = folder;
	g_free(folder);
	return f;
}

String ExportDialog::getFilename()
{
	XOJ_CHECK_TYPE(ExportDialog);

	return gtk_entry_get_text(GTK_ENTRY(get("txtFilename")));
}

bool ExportDialog::validate()
{
	XOJ_CHECK_TYPE(ExportDialog);

	if (gtk_entry_get_text_length(GTK_ENTRY(get("txtFilename"))) == 0)
	{
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
		                                           GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		                                           _("The filename should not be empty"));

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		return false;
	}

	GtkWidget* rdFormatPDF = get("rdFormatPDF");
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rdFormatPDF)))
	{
		GtkFileChooser* chooser = GTK_FILE_CHOOSER(get("fcOutput"));
		char* folder = gtk_file_chooser_get_current_folder(chooser);
		String path = folder;
		g_free(folder);

		path += G_DIR_SEPARATOR_S;

		const char* txtFilename = gtk_entry_get_text(GTK_ENTRY(get("txtFilename")));
		path += txtFilename;

		if (g_file_test(path.c_str(), G_FILE_TEST_EXISTS))
		{
			GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
			                                           GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
			                                           _("The file already exists."));

			gtk_dialog_add_button(GTK_DIALOG(dialog), _("Cancel"), 2);
			gtk_dialog_add_button(GTK_DIALOG(dialog), _("Replace PDF"), 1);
			int res = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);

			if (res != 1)
			{
				return false;
			}
		}
	}
	else
	{
		GtkFileChooser* chooser = GTK_FILE_CHOOSER(get("fcOutput"));
		char* folder = gtk_file_chooser_get_current_folder(chooser);
		GFile* file = g_file_new_for_path(folder);
		g_free(folder);
		GFileEnumerator* enumerator = g_file_enumerate_children(file, "standard::*",
		                                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, NULL);
		g_object_unref(file);

		if (enumerator != NULL)
		{
			GFileInfo* info = g_file_enumerator_next_file(enumerator, NULL, NULL);
			if (info)
			{
				g_object_unref(info);

				GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *this,
				                                           GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
				                                           _("The folder is not empty. May existing files will be overwritten."));

				gtk_dialog_add_button(GTK_DIALOG(dialog), _("Select another folder"), 2);
				gtk_dialog_add_button(GTK_DIALOG(dialog), _("Continue anyway"), 1);
				int res = gtk_dialog_run(GTK_DIALOG(dialog));
				gtk_widget_destroy(dialog);

				if (res != 1)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void ExportDialog::show(GtkWindow* parent)
{
	XOJ_CHECK_TYPE(ExportDialog);

	int res = 0;
	do
	{
		gtk_window_set_transient_for(GTK_WINDOW(this->window), parent);
		res = gtk_dialog_run(GTK_DIALOG(this->window));
		if (res == 2)
		{
			if (validate())
			{
				handleData();
				break;
			}
		}
	}
	while (res == 2);

	gtk_widget_hide(this->window);
}

