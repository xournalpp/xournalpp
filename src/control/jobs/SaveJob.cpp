#include "SaveJob.h"

//workaround for filesystem::copy_file (see http://polr.me/1db )
//marked for removal after upgrade to boost 1.57
#define BOOST_NO_CXX11_SCOPED_ENUMS

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"
#include "view/DocumentView.h"

#include <config.h>
#include <i18n.h>

#include <boost/filesystem/operations.hpp>

SaveJob::SaveJob(Control* control)
 : BlockingJob(control, _("Save"))
{
	XOJ_INIT_TYPE(SaveJob);
}

SaveJob::~SaveJob()
{
	XOJ_RELEASE_TYPE(SaveJob);
}

void SaveJob::run()
{
	XOJ_CHECK_TYPE(SaveJob);

	save();

	if (this->control->getWindow())
	{
		callAfterRun();
	}
}

void SaveJob::afterRun()
{
	XOJ_CHECK_TYPE(SaveJob);

	if (!this->lastError.empty())
	{
		Util::showErrorToUser(control->getGtkWindow(), this->lastError);
	}
	else
	{
		Document* doc = this->control->getDocument();

		doc->lock();
		path filename = doc->getFilename();
		doc->unlock();

		control->getUndoRedoHandler()->documentSaved();
		control->getRecentManager()->addRecentFileFilename(filename);
		control->updateWindowTitle();
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
		doc->setPreview(NULL);
	}

	doc->unlock();
}

bool SaveJob::save()
{
	XOJ_CHECK_TYPE(SaveJob);

	updatePreview(control);
	Document* doc = this->control->getDocument();

	SaveHandler h;

	doc->lock();
	h.prepareSave(doc);
	path filename = doc->getFilename();
	doc->unlock();

	if (doc->shouldCreateBackupOnSave())
	{
		path backup = filename.parent_path();
		backup /= std::string(".") + filename.filename().replace_extension(".xopp.bak").string();

		using namespace boost::filesystem;
		try
		{
			copy_file(doc->getFilename(), backup, copy_option::overwrite_if_exists);
		}
		catch (const filesystem_error& e)
		{
			g_warning("%s\n%s", _C("Could not create backup! (The file was created from an older Xournal version)"), e.what());
		}

		doc->setCreateBackupOnSave(false);
	}

	doc->lock();

	h.saveTo(filename, this->control);
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
