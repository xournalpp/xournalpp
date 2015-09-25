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

SaveJob::SaveJob(Control* control) : BlockingJob(control, _("Save"))
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
		GtkWidget* dialog = gtk_message_dialog_new((GtkWindow*) *control->getWindow(),
												   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
												   "%s", this->lastError.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
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

void SaveJob::updatePreview()
{
	XOJ_CHECK_TYPE(SaveJob);

	const int previewSize = 128;

	Document* doc = this->control->getDocument();

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

		if (page->getBackgroundType() == BACKGROUND_TYPE_PDF)
		{
			int pgNo = page->getPdfPageNr();
			XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);
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

	updatePreview();
	Document* doc = this->control->getDocument();

	SaveHandler h;

	doc->lock();
	h.prepareSave(doc);
	path filename = doc->getFilename();
	doc->unlock();

	if (doc->shouldCreateBackupOnSave())
	{
		path backup = filename.parent_path();
		backup /= CONCAT(".", filename.filename().replace_extension(".xoj.bak").string());

		using namespace boost::filesystem;
		try
		{
			copy_file(doc->getFilename(), backup, copy_option::overwrite_if_exists);
		}
		catch (const filesystem_error& e)
		{
			g_warning("Could not create backup! (The file was created from an older Xournal version)\n%s\n", e.what());
		}

		doc->setCreateBackupOnSave(false);
	}

	GzOutputStream* out = new GzOutputStream(filename);

	if (!out->getLastError().empty())
	{
		if (!control->getWindow())
		{
			g_error(_C("Open file error: %s"), out->getLastError().c_str());
			return false;
		}

		this->lastError = (bl::format("Open file error: {1}") % out->getLastError()).str();

		delete out;
		out = NULL;
		return false;
	}

	h.saveTo(out, filename, this->control);
	out->close();

	if (!out->getLastError().empty())
	{
		this->lastError = (bl::format("Open file error: {1}") % out->getLastError()).str();
		if (!control->getWindow())
		{
			g_error("%s", this->lastError.c_str());
			return false;
		}

		delete out;
		out = NULL;
		return false;
	}

	delete out;
	out = NULL;
	return true;
}
