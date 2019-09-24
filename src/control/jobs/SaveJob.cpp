#include "SaveJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"
#include "view/DocumentView.h"

#include <config.h>
#include <i18n.h>
#include <XojMsgBox.h>


SaveJob::SaveJob(Control* control)
 : BlockingJob(control, _("Save"))
{
}

SaveJob::~SaveJob()
{
}

void SaveJob::run()
{
	save();

	if (this->control->getWindow())
	{
		callAfterRun();
	}
}

void SaveJob::afterRun()
{
	if (!this->lastError.empty())
	{
		XojMsgBox::showErrorToUser(control->getGtkWindow(), this->lastError);
	}
	else
	{
		this->control->resetSavedStatus();
	}
}

void SaveJob::updatePreview(Control* control)
{
	const int previewSize = 128;

	Document* doc = control->getDocument();

	doc->lock();

	if (doc->getPageCount() > 0)
	{
		PageRef page = doc->getPage(0);

		double width = page->getWidth();
		double height = page->getHeight();

		double zoom = 1;

		if (width < height)
		{
			zoom = previewSize / height;
		}
		else
		{
			zoom = previewSize / width;
		}
		width *= zoom;
		height *= zoom;

		cairo_surface_t* crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

		cairo_t* cr = cairo_create(crBuffer);
		cairo_scale(cr, zoom, zoom);

		if (page->getBackgroundType().isPdfPage())
		{
			int pgNo = page->getPdfPageNr();
			XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);
			if (popplerPage)
			{
				popplerPage->render(cr, false);
			}
		}

		DocumentView view;
		view.drawPage(page, cr, true);
		cairo_destroy(cr);
		doc->setPreview(crBuffer);
		cairo_surface_destroy(crBuffer);
	}
	else
	{
		doc->setPreview(nullptr);
	}

	doc->unlock();
}

bool SaveJob::save()
{
	updatePreview(control);
	Document* doc = this->control->getDocument();

	SaveHandler h;

	doc->lock();
	h.prepareSave(doc);
	Path filename = doc->getFilename();
	filename.clearExtensions();
	filename += ".xopp";
	doc->unlock();

	if (doc->shouldCreateBackupOnSave())
	{
		Path backup = filename;
		backup += "~";

		if (!PathUtil::copy(doc->getFilename(), backup))
		{
			g_warning(_("Could not create backup! (The file was created from an older Xournal version)"));
		}

		doc->setCreateBackupOnSave(false);
	}

	doc->lock();

	h.saveTo(filename, this->control);
	doc->setFilename(filename);
	doc->unlock();

	if (!h.getErrorMessage().empty())
	{
		this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());
		if (!control->getWindow())
		{
			g_error("%s", this->lastError.c_str());
			return false;
		}

		return false;
	}

	return true;
}
