#include "AutosaveJob.h"

#include "control/Control.h"
#include "control/SaveHandler.h"

AutosaveJob::AutosaveJob(Control* control)
{
	XOJ_INIT_TYPE(AutosaveJob);

	this->control = control;
}

AutosaveJob::~AutosaveJob()
{
	XOJ_RELEASE_TYPE(AutosaveJob);
}

void AutosaveJob::afterRun()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) control->getWindow(),
											   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Autosave: %s"),
											   this->error.c_str());
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void AutosaveJob::run()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	SaveHandler handler;

	control->getUndoRedoHandler()->documentAutosaved();

	Document* doc = control->getDocument();

	doc->lock();
	handler.prepareSave(doc);
	path filename = doc->getFilename();
	doc->unlock();

	if (filename.empty())
	{
		filename = Util::getAutosaveFilename();
	}
	else
	{
		string file = filename.filename().string();
		filename.remove_filename();
		filename /= CONCAT(".", file);
		filename.replace_extension(".autosave.xoj");
	}

	control->renameLastAutosaveFile();

	GzOutputStream* out = new GzOutputStream(filename);
	handler.saveTo(out, filename);
	delete out;
	out = NULL;

	this->error = handler.getErrorMessage();
	if (!this->error.empty())
	{
		callAfterRun();
	}
	else
	{
		//control->deleteLastAutosaveFile(filename);
		control->setLastAutosaveFile(filename);
	}
}

JobType AutosaveJob::getType()
{
	XOJ_CHECK_TYPE(AutosaveJob);
	return JOB_TYPE_AUTOSAVE;
}

