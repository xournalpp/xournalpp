#include "CustomExportJob.h"
#include "SynchronizedProgressListener.h"

#include "control/Control.h"
#include "gui/dialog/ExportDialog.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "view/PdfView.h"

#include <i18n.h>


CustomExportJob::CustomExportJob(Control* control)
 : BaseExportJob(control, _("Custom Export")),
   exportTypePdf(false),
   pngDpi(300),
   surface(NULL),
   cr(NULL)
{
	XOJ_INIT_TYPE(CustomExportJob);
}

CustomExportJob::~CustomExportJob()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	for (PageRangeEntry* e : exportRange)
	{
		delete e;
	}
	exportRange.clear();


	XOJ_RELEASE_TYPE(CustomExportJob);
}

void CustomExportJob::addFilterToDialog()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	addFileFilterToDialog(_C("PDF files"), "*.pdf");
	addFileFilterToDialog(_C("PNG graphics"), "*.png");
}

bool CustomExportJob::isUriValid(string& uri)
{
	XOJ_CHECK_TYPE(CustomExportJob);

	if (!BaseExportJob::isUriValid(uri))
	{
		return false;
	}

	string ext = filename.extension().string();
	if (ext != ".pdf" && ext != ".png")
	{
		string msg = _C("File name needs to end with .pdf or .png");
		GtkWindow* win = (GtkWindow*) *control->getWindow();
		GtkWidget* dialog = gtk_message_dialog_new(win, GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg.c_str());
		gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return false;
	}

	return true;
}

bool CustomExportJob::showFilechooser()
{
	if (!BaseExportJob::showFilechooser())
	{
		return false;
	}

	Document* doc = control->getDocument();
	doc->lock();

	ExportDialog* dlg = new ExportDialog(control->getGladeSearchPath());

	string ext = filename.extension().string();
	if (ext == ".pdf")
	{
		dlg->removeDpiSelection();
		exportTypePdf = true;
	}

	dlg->initPages(control->getCurrentPageNo() + 1, doc->getPageCount());

	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

	if (!dlg->isConfirmed())
	{
		return false;
	}

	exportRange = dlg->getRange();
	pngDpi = dlg->getPngDpi();

	delete dlg;
	doc->unlock();
	return true;
}

void CustomExportJob::createSurface(double width, double height)
{
	XOJ_CHECK_TYPE(CustomExportJob);

	this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
											   width * this->pngDpi / 72.0,
											   height * this->pngDpi / 72.0);
	this->cr = cairo_create(this->surface);
	double factor = this->pngDpi / 72.0;
	cairo_scale(this->cr, factor, factor);
}

bool CustomExportJob::freeSurface(int id)
{
	XOJ_CHECK_TYPE(CustomExportJob);

	cairo_destroy(this->cr);

	string filepath = getFilenameWithNumber(id);
	cairo_status_t status = cairo_surface_write_to_png(surface, filepath.c_str());
	cairo_surface_destroy(surface);

	// we ignore this problem
	if (status != CAIRO_STATUS_SUCCESS)
	{
		return false;
	}

	return true;
}

/**
 * Get a filename with a numer, e.g. .../export-1.png, if the no is -1, return .../export.png
 */
string CustomExportJob::getFilenameWithNumber(int no)
{
	if (no == -1)
	{
		// No number to add
		return filename.c_str();
	}

	string filepath = filename.c_str();
	size_t dotPos = filepath.find_last_of(".");
	if (dotPos == string::npos)
	{
		// No file extension, add number
		return filepath + "-" + std::to_string(no);
	}

	return filepath.substr(0, dotPos) + "-" + std::to_string(no) + filepath.substr(dotPos);
}

/**
 * Export a single PNG page
 */
void CustomExportJob::exportPngPage(int pageId, int id, double zoom, DocumentView& view)
{
	Document* doc = control->getDocument();
	doc->lock();
	PageRef page = doc->getPage(pageId);
	doc->unlock();

	createSurface(page->getWidth(), page->getHeight());

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
		this->errorMsg = _("Error export PDF Page");
		return;
	}
}

/**
 * Create one PNG file per page
 */
void CustomExportJob::exportPng()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	SynchronizedProgressListener pglistener(this->control);
	// don't lock the page here for the whole flow, else we get a dead lock...
	// the ui is blocked, so there should be no changes...
	Document* doc = control->getDocument();

	int count = doc->getPageCount();

	bool onePage = ((this->exportRange.size() == 1) && (this->exportRange[0]->getFirst() == this->exportRange[0]->getLast()));

	char selectedPages[count];
	int selectedCount = 0;
	for (int i = 0; i < count; i++)
	{
		selectedPages[i] = 0;
	}
	for (PageRangeEntry* e : this->exportRange)
	{
		for (int x = e->getFirst(); x <= e->getLast(); x++)
		{
			selectedPages[x] = 1;
			selectedCount++;
		}
	}

	pglistener.setMaximumState(selectedCount);

	DocumentView view;
	double zoom = this->pngDpi / 72.0;
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
			exportPngPage(i, id, zoom, view);
		}
	}
}

void CustomExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(CustomExportJob);

	// pdf, supports multiple Pages per document, all other formats don't
	if (exportTypePdf)
	{
		SynchronizedProgressListener pglistener(this->control);
		// don't lock the page here for the whole flow, else we get a dead lock...
		// the ui is blocked, so there should be no changes...
		Document* doc = control->getDocument();

		PdfExport pdfe(doc, &pglistener);

		if (!pdfe.createPdf(this->filename, exportRange))
		{
			this->errorMsg = pdfe.getLastError();
		}
	}
	else
	{
		exportPng();
	}
}
