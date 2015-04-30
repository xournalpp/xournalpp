#include "ExportJob.h"

#include "control/Control.h"
#include "ExportFormtType.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "SynchronizedProgressListener.h"
#include "view/DocumentView.h"
#include "view/PdfView.h"

#include <config.h>

#include <cairo-ps.h>
#include <cairo-svg.h>

#include <glib/gi18n-lib.h>

ExportJob::ExportJob(Control* control, PageRangeVector selected, ExportFormtType type, int dpi, path filepath) :
	BlockingJob(control, _("Export"))
{
	XOJ_INIT_TYPE(ExportJob);

	this->selected = selected;

	this->surface = NULL;
	this->cr = NULL;
	this->type = type;
	this->dpi = dpi;

	this->filepath = filepath;

	string filename = filepath.filename().string();
	int index = filename.find_last_of(".");
	if (index == 0 || index == string::npos)
	{
		front = filename;
	}
	else
	{
		front = filename.substr(0, index);
		back = filename.substr(index + 1);
	}

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

bool ExportJob::createSurface(int id, double width, double height)
{
	XOJ_CHECK_TYPE(ExportJob);

	if (this->type == EXPORT_FORMAT_EPS)
	{
		path filepath;
		if (id == -1)
		{
			filepath = this->filepath.parent_path();
			filepath /= this->front;
		}
		else
		{
			filepath = this->filepath;
			filepath += std::to_string(id);
		}
		filepath += '.' + this->back;

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
		path filepath = this->filepath.parent_path();
		filepath /= this->front;
		if (id != -1)
		{
			filepath += std::to_string(id);
		}
		filepath += CONCAT('.', this->back);

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
		path filepath = this->filepath;
		filepath /= this->front;
		if (id != -1) filepath += std::to_string(id);
		filepath += CONCAT('.', this->back);

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

void ExportJob::run()
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


					// TODO LOW PRIO pdf is written as image to the SVN surface!!
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
