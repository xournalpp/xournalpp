#include "BaseExportJob.h"

#include "control/Control.h"

#include <i18n.h>
#include <XojMsgBox.h>

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <boost/algorithm/string.hpp>
namespace ba = boost::algorithm;

BaseExportJob::BaseExportJob(Control* control, string name)
 : BlockingJob(control, name),
   dialog(NULL)
{
	XOJ_INIT_TYPE(BaseExportJob);
}

BaseExportJob::~BaseExportJob()
{
	XOJ_CHECK_TYPE(BaseExportJob);

	XOJ_RELEASE_TYPE(BaseExportJob);
}

void BaseExportJob::initDialog()
{
	dialog = gtk_file_chooser_dialog_new(_("Export PDF"), control->getGtkWindow(), GTK_FILE_CHOOSER_ACTION_SAVE,
													_("_Cancel"), GTK_RESPONSE_CANCEL,
													_("_Save"), GTK_RESPONSE_OK, NULL);

	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
}

void BaseExportJob::addFileFilterToDialog(string name, string pattern)
{
	GtkFileFilter* filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, name.c_str());
	gtk_file_filter_add_pattern(filter, pattern.c_str());
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}

void BaseExportJob::clearExtensions(path& filename)
{
	XOJ_CHECK_TYPE(BaseExportJob);
	while (filename.has_extension())
	{
		filename.replace_extension();
	}
}

bool BaseExportJob::checkOverwriteBackgroundPDF(path& filename)
{
	XOJ_CHECK_TYPE(BaseExportJob);
	
	// If the new file name (with the selected extension) is the previously selected pdf, warn the user
	if (boost::iequals(filename.string(), control->getDocument()->getPdfFilename().string()))
	{
		string msg = _("Do not overwrite the background PDF! This will cause errors!");
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}
	return true;
}

string BaseExportJob::getFilterName()
{
	XOJ_CHECK_TYPE(BaseExportJob);
	
	GtkFileFilter* filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
	return gtk_file_filter_get_name(filter);
}

bool BaseExportJob::showFilechooser()
{
	XOJ_CHECK_TYPE(BaseExportJob);

	initDialog();
	addFilterToDialog();

	Settings* settings = control->getSettings();
	Document* doc = control->getDocument();
	doc->lock();
	path folder = doc->createSaveFolder(settings->getLastSavePath());
	path name = doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName());
	doc->unlock();

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), PATH_TO_CSTR(folder));
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), PATH_TO_CSTR(name));
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));

	while (true)
	{
		if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
		{
			gtk_widget_destroy(dialog);
			return false;
		}

		string uri(gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));
		this->filename = path(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))).replace_extension();
		path currentFolder(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog)));

		// Since we add the extension after the OK button, we have to check manually on existing files
		if (isUriValid(uri) && control->checkExistingFile(currentFolder, filename))
		{
			break;
		}
	}

	settings->setLastSavePath(this->filename.parent_path());

	gtk_widget_destroy(dialog);

	return true;
}

bool BaseExportJob::isUriValid(string& uri)
{
	XOJ_CHECK_TYPE(BaseExportJob);

	if (!ba::starts_with(uri, "file://"))
	{
		string msg = FS(_F("Only local files are supported\nPath: {1}") % uri);
		XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}

	return true;
}

void BaseExportJob::afterRun()
{
	XOJ_CHECK_TYPE(BaseExportJob);

	if (!this->errorMsg.empty())
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg);
	}
}

