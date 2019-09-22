#include "AutosaveJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"

#include <i18n.h>
#include <Path.h>
#include <XojMsgBox.h>

AutosaveJob::AutosaveJob(Control* control)
 : control(control)
{
}

AutosaveJob::~AutosaveJob()
{
}

void AutosaveJob::afterRun()
{
	string msg = FS(_F("Error while autosaving: {1}") % this->error);
	g_warning("%s", msg.c_str());
	XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
}

void AutosaveJob::run()
{
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
	}
	filename.clearExtensions();
	filename += ".autosave.xopp";

	control->renameLastAutosaveFile();

	g_message("%s", FS(_F("Autosaving to {1}") % filename.str()).c_str());

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
	return JOB_TYPE_AUTOSAVE;
}

