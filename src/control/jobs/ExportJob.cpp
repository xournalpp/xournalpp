#include "ExportJob.h"

#include "ExportFormtType.h"
#include "SynchronizedProgressListener.h"

#include "control/Control.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

#include <config.h>
#include <i18n.h>

#include <cairo-ps.h>
#include <cairo-svg.h>

ExportJob::ExportJob(Control* control, PageRangeVector selected, ExportFormtType type, int dpi, string filepath)
 : BlockingJob(control, _("Export")),
   selected(selected),
   surface(NULL),
   cr(NULL),
   type(type),
   dpi(dpi),
   filepath(filepath)
{
	XOJ_INIT_TYPE(ExportJob);
}

ExportJob::~ExportJob()
{
	XOJ_CHECK_TYPE(ExportJob);

	for (PageRangeEntry* e : this->selected)
	{
		delete e;
		e = NULL;
	}

	XOJ_RELEASE_TYPE(ExportJob);
}


/**
 * Get a filename with a numer, e.g. .../export-1.png, if the no is -1, return .../export.png
 */
string ExportJob::getFilenameWithNumber(int no)
{
	if (no == -1)
	{
		// No number to add
		return filepath;
	}

	size_t dotPos = filepath.find_last_of(".");
	if (dotPos == string::npos)
	{
		// No file extension, add number
		return filepath + "-" + std::to_string(no);
	}

	return filepath.substr(0, dotPos) + "-" + std::to_string(no) + filepath.substr(dotPos);
}

bool ExportJob::createSurface(int id, double width, double height)
{
	XOJ_CHECK_TYPE(ExportJob);

	if (this->type == EXPORT_FORMAT_EPS)
	{
		string filepath = getFilenameWithNumber(id);
		this->surface = cairo_ps_surface_create(filepath.c_str(), width, height);

		cairo_ps_surface_set_eps(this->surface, true);

		this->cr = cairo_create(this->surface);
	}
	else if (this->type == EXPORT_FORMAT_PNG)
	{
		this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
												   width * this->dpi / 72.0,
												   height * this->dpi / 72.0);
		this->cr = cairo_create(this->surface);
		double factor = this->dpi / 72.0;
		cairo_scale(this->cr, factor, factor);
	}
	else if (this->type == EXPORT_FORMAT_SVG)
	{
		string filepath = getFilenameWithNumber(id);
		this->surface = cairo_svg_surface_create(filepath.c_str(), width, height);

		this->cr = cairo_create(this->surface);
	}
	else
	{
		g_error("ExportHandler::createSurface unknown ExportFormtType %i", this->type);
		return false;
	}

	return true;
}

bool ExportJob::freeSurface(int id)
{
	XOJ_CHECK_TYPE(ExportJob);

	cairo_destroy(this->cr);

	if (this->type == EXPORT_FORMAT_PNG)
	{
		string filepath = getFilenameWithNumber(id);
		cairo_status_t status = cairo_surface_write_to_png(surface, filepath.c_str());
		cairo_surface_destroy(surface);

		// we ignore this problem
		if (status != CAIRO_STATUS_SUCCESS)
		{
			return false;
		}
	}
	else
	{
		cairo_surface_destroy(this->surface);
	}
	return true;
}

void ExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(ExportJob);

	SynchronizedProgressListener pglistener(this->control);
	Document* doc = control->getDocument();

	// don't lock the page here for the whole flow, else we get a dead lock...
	// the ui is blocked, so there should be no changes...

	int count = doc->getPageCount();

	bool onePage = ((this->selected.size() == 1) && (this->selected[0]->getFirst() == this->selected[0]->getLast()));

	// pdf, supports multiple Pages per document, all other formats don't
	if (this->type == EXPORT_FORMAT_PDF)
	{
		PdfExport pdfe(doc, &pglistener);

		if (!pdfe.createPdf(this->filepath))
		{
			g_warning("Error creating PDF: %s", pdfe.getLastError().c_str());
		}
	}
	else // all other formats need one file per page
	{
		char selectedPages[count];
		int selectedCount = 0;
		for (int i = 0; i < count; i++)
		{
			selectedPages[i] = 0;
		}
		for (PageRangeEntry* e : this->selected)
		{
			for (int x = e->getFirst(); x <= e->getLast(); x++)
			{
				selectedPages[x] = 1;
				selectedCount++;
			}
		}

		pglistener.setMaximumState(selectedCount);

		DocumentView view;
		double zoom = 1;

		if (this->type == EXPORT_FORMAT_PNG)
		{
			zoom = this->dpi / 72.0;
		}
		int current = 0;

		for (int i = 0; i < count; i++)
		{
			int id = i + 1;
			if (onePage)
			{
				id = -1;
			}

			if (selectedPages[i])
			{
				pglistener.setCurrentState(current++);

				doc->lock();
				PageRef page = doc->getPage(i);
				doc->unlock();

				if (!createSurface(id, page->getWidth(), page->getHeight()))
				{
					// could not create this file...
					continue;
				}

				if (page->getBackgroundType() == BACKGROUND_TYPE_PDF)
				{
					int pgNo = page->getPdfPageNr();
					XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);

					PdfView::drawPage(NULL, popplerPage, cr, zoom, page->getWidth(), page->getHeight());
				}

				view.drawPage(page, this->cr, true);

				if (!freeSurface(id))
				{
					// could not create this file...
					continue;
				}
			}
		} // end for loop
	}
}
