#include "ExportJob.h"
#include "../../view/DocumentView.h"
#include "../../view/PdfView.h"
#include <PageRange.h>
#include "SynchronizedProgressListener.h"
#include <cairo-ps.h>
#include <cairo-svg.h>
#include "../Control.h"
#include "ExportFormtType.h"
#include "../../pdf/popplerdirect/PdfExport.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ExportJob::ExportJob(Control* control, GList* selected, ExportFormtType type,
                     int dpi, String folder, String filename) :
	BlockingJob(control, _("Export"))
{
	XOJ_INIT_TYPE(ExportJob);

	this->selected = selected;

	this->surface = NULL;
	this->cr = NULL;
	this->type = type;
	this->dpi = dpi;

	this->folder = folder;
	this->filename = filename;
}

ExportJob::~ExportJob()
{
	XOJ_CHECK_TYPE(ExportJob);

	for (GList* l = this->selected; l != NULL; l = l->next)
	{
		PageRangeEntry* e = (PageRangeEntry*) l->data;
		delete e;
	}
	g_list_free(this->selected);
	this->selected = NULL;

	XOJ_RELEASE_TYPE(ExportJob);
}

bool ExportJob::createSurface(int id, double width, double height)
{
	XOJ_CHECK_TYPE(ExportJob);

	if (this->type == EXPORT_FORMAT_EPS)
	{
		char* path = NULL;
		if (id == -1)
		{
			path = g_strdup_printf("%s%c%s.eps", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str());
		}
		else
		{
			path = g_strdup_printf("%s%c%s%i.eps", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str(), id);
		}

		this->surface = cairo_ps_surface_create(path, width, height);
		g_free(path);

		cairo_ps_surface_set_eps(this->surface, true);

		this->cr = cairo_create(this->surface);
	}
	else if (this->type == EXPORT_FORMAT_PNG)
	{
		this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
		                                           width * this->dpi / 72.0, height * this->dpi / 72.0);
		this->cr = cairo_create(this->surface);
		double factor = this->dpi / 72.0;
		cairo_scale(this->cr, factor, factor);
	}
	else if (this->type == EXPORT_FORMAT_SVG)
	{
		char* path = NULL;
		if (id == -1)
		{
			path = g_strdup_printf("%s%c%s.svg", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str());
		}
		else
		{
			path = g_strdup_printf("%s%c%s%i.svg", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str(), id);
		}

		this->surface = cairo_svg_surface_create(path, width, height);
		g_free(path);

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
		char* path = NULL;
		if (id == -1)
		{
			path = g_strdup_printf("%s%c%s.png", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str());
		}
		else
		{
			path = g_strdup_printf("%s%c%s%i.png", this->folder.c_str(), G_DIR_SEPARATOR,
			                       this->filename.c_str(), id);
		}

		cairo_status_t status = cairo_surface_write_to_png(surface, path);
		cairo_surface_destroy(surface);

		// we ignore this problem
		if (status != CAIRO_STATUS_SUCCESS)
		{
			return false;
		}

		g_free(path);
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

	bool onePage = false;
	if (this->selected->next == NULL)
	{
		PageRangeEntry* e = (PageRangeEntry*) selected->data;
		if (e->getFirst() == e->getLast())
		{
			onePage = true;
		}
	}

	// pdf, supports multiple Pages per document, all other formats don't
	if (this->type == EXPORT_FORMAT_PDF)
	{
		PdfExport pdfe(doc, &pglistener);
		char* path = g_strdup_printf("file://%s%c%s.pdf", this->folder.c_str(),
		                             G_DIR_SEPARATOR, this->filename.c_str());

		if(!pdfe.createPdf(path))
		{
			g_warning("Error creating PDF: %s", pdfe.getLastError().c_str());
		}

		g_free(path);
	}
	else     // all other formats need one file per page
	{
		char selectedPages[count];
		int selectedCount = 0;
		for (int i = 0; i < count; i++)
		{
			selectedPages[i] = 0;
		}
		for (GList* l = this->selected; l != NULL; l = l->next)
		{
			PageRangeEntry* e = (PageRangeEntry*) l->data;
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

				if (!createSurface(id, page.getWidth(), page.getHeight()))
				{
					// could not create this file...
					continue;
				}

				if (page.getBackgroundType() == BACKGROUND_TYPE_PDF)
				{
					int pgNo = page.getPdfPageNr();
					XojPopplerPage* popplerPage = doc->getPdfPage(pgNo);


					// TODO LOW PRIO pdf is written as image to the SVN surface!!
					PdfView::drawPage(NULL, popplerPage, cr, zoom, page.getWidth(),
					                  page.getHeight());
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
