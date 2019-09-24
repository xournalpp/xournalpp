#include "BaseExportJob.h"

#include "control/Control.h"

#include <i18n.h>
#include <StringUtils.h>
#include <XojMsgBox.h>

BaseExportJob::BaseExportJob(Control* control, string name)
 : BlockingJob(control, name)
{
}

BaseExportJob::~BaseExportJob()
{
}

void BaseExportJob::initDialog()
{
	dialog = gtk_file_chooser_dialog_new(_("Export PDF"), control->getGtkWindow(), GTK_FILE_CHOOSER_ACTION_SAVE,
													_("_Cancel"), GTK_RESPONSE_CANCEL,
													_("_Save"), GTK_RESPONSE_OK, nullptr);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
}

void BaseExportJob::addFileFilterToDialog(string name, string pattern)
{
	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, name.c_str());
	gtk_file_filter_add_pattern(filter, pattern.c_str());
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}

bool BaseExportJob::checkOverwriteBackgroundPDF(Path& filename)
{
	// If the new file name (with the selected extension) is the previously selected pdf, warn the user
	if (StringUtils::iequals(filename.str(), control->getDocument()->getPdfFilename().str()))
	{
		string msg = _("Do not overwrite the background PDF! This will cause errors!");
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}
	return true;
}

string BaseExportJob::getFilterName()
{
	GtkFileFilter* filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
	return gtk_file_filter_get_name(filter);
}

bool BaseExportJob::showFilechooser()
{
	initDialog();
	addFilterToDialog();

	Settings* settings = control->getSettings();
	Document* doc = control->getDocument();
	doc->lock();
	Path folder = doc->createSaveFolder(settings->getLastSavePath());
	Path name = doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName());
	doc->unlock();

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder.c_str());
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), name.c_str());
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));

	while (true)
	{
		if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
		{
			gtk_widget_destroy(dialog);
			return false;
		}

		string uri(gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));
		this->filename = Path(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
		this->filename.clearExtensions();
		Path currentFolder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

		// Since we add the extension after the OK button, we have to check manually on existing files
		if (isUriValid(uri) && control->checkExistingFile(currentFolder, filename))
		{
			break;
		}
	}

	settings->setLastSavePath(this->filename.getParentPath());

	gtk_widget_destroy(dialog);

	return true;
}

bool BaseExportJob::isUriValid(string& uri)
{
	if (!StringUtils::startsWith(uri, "file://"))
	{
		string msg = FS(_F("Only local files are supported\nPath: {1}") % uri);
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}

	return true;
}

void BaseExportJob::afterRun()
{
	if (!this->errorMsg.empty())
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg);
	}
}

