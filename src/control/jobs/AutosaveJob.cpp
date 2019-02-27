#include "AutosaveJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"

#include <i18n.h>
#include <Path.h>
#include <XojMsgBox.h>

AutosaveJob::AutosaveJob(Control* control)
 : control(control)
{
	XOJ_INIT_TYPE(AutosaveJob);
}

AutosaveJob::~AutosaveJob()
{
	XOJ_RELEASE_TYPE(AutosaveJob);
}

void AutosaveJob::afterRun()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	string msg = FS(_F("Autosave: {1}") % this->error);
	XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
}

void AutosaveJob::run()
{
	XOJ_CHECK_TYPE(AutosaveJob);

	SaveHandler handler;

	control->getUndoRedoHandler()->documentAutosaved();

	Document* doc = control->getDocument();

	doc->lock();
	handler.prepareSave(doc);
	Path filename = doc->getFilename();
	doc->unlock();

	if (filename.isEmpty())
	{
		filename = Util::getAutosaveFilename();
	}
	else
	{
		string file = filename.getFilename();
		filename = filename.getParentPath();
		filename /= string(".") + file;
		filename.clearExtensions();
		filename += ".autosave.xopp";
	}

	control->renameLastAutosaveFile();

	handler.saveTo(filename);

	this->error = handler.getErrorMessage();
	if (!this->error.empty())
	{
		callAfterRun();
	}
	else
	{
		// control->deleteLastAutosaveFile(filename);
		control->setLastAutosaveFile(filename);
	}
}

JobType AutosaveJob::getType()
{
	XOJ_CHECK_TYPE(AutosaveJob);
	return JOB_TYPE_AUTOSAVE;
}

