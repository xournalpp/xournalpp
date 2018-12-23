#include "CustomExportJob.h"
#include "SaveJob.h"

#include "control/Control.h"
#include "control/xojfile/XojExportHandler.h"
#include "gui/dialog/ExportDialog.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "view/PdfView.h"

#include <i18n.h>
#include <config-features.h>

CustomExportJob::CustomExportJob(Control* control)
 : BaseExportJob(control, _("Custom Export")),
   pngDpi(300),
   surface(NULL),
   cr(NULL),
   exportTypePdf(false),
   exportTypeXoj(false)
{
	XOJ_INIT_TYPE(CustomExportJob);

	// Supported filters
	filters[_("PDF files")] = ".pdf"; 
	filters[_("PDF with plain background")] = ".pdf";
	filters[_("PNG graphics")] = ".png";
	filters[_("PNG with transparent background")] = ".png";
	filters[_("Xournal (Compatibility)")] =  ".xoj";
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

	// Runs on every filter inside the filters map
	for (std::map<string, string>::iterator it = filters.begin(); it != filters.end(); ++it)
	{
  		addFileFilterToDialog(it->first, "*" + it->second); // Adds * for the pattern
	}
}

bool CustomExportJob::isUriValid(string& uri)
{
	XOJ_CHECK_TYPE(CustomExportJob);

	if (!BaseExportJob::isUriValid(uri))
	{
		return false;
	}

	// Extract the file filter selected	
	this->chosenFilterName = BaseExportJob::getFilterName();
	
	// Remove any pre-existing extension and adds the chosen one
	clearExtensions(filename);
	filename.replace_extension(filters[this->chosenFilterName]);

	return checkOverwriteBackgroundPDF(filename);
}

bool CustomExportJob::showFilechooser()
{
	if (!BaseExportJob::showFilechooser())
	{
		return false;
	}

	string ext = filename.extension().string();
	if (ext == ".xoj")
	{
		exportTypeXoj = true;
		return true;
	}

	Document* doc = control->getDocument();
	doc->lock();
	ExportDialog* dlg = new ExportDialog(control->getGladeSearchPath());
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
		return filename.string();
	}

	string filepath = filename.string();
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

	if (page->getBackgroundType().isPdfPage())
	{
		int pgNo = page->getPdfPageNr();
		XojPdfPageSPtr popplerPage = doc->getPdfPage(pgNo);

		PdfView::drawPage(NULL, popplerPage, cr, zoom, page->getWidth(), page->getHeight());
	}

	if (this->chosenFilterName == _("PNG with transparent background"))
	{
		view.drawPage(page, this->cr, true, true);
	}
	else
	{
		view.drawPage(page, this->cr, true);
	}

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

	this->control->setMaximumState(selectedCount);

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
			this->control->setCurrentState(current++);
			exportPngPage(i, id, zoom, view);
		}
	}
}

void CustomExportJob::run()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	if (exportTypeXoj)
	{
		SaveJob::updatePreview(control);
		Document* doc = this->control->getDocument();

		XojExportHandler h;
		doc->lock();
		h.prepareSave(doc);
		h.saveTo(filename, this->control);
		doc->unlock();

		if (!h.getErrorMessage().empty())
		{
			this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());

			callAfterRun();
		}
	}
	else if (exportTypePdf)
	{
		// don't lock the page here for the whole flow, else we get a dead lock...
		// the ui is blocked, so there should be no changes...
		Document* doc = control->getDocument();

		XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, control);
		
		

		if (this->chosenFilterName == _("PDF with plain background"))
		{
			pdfe->setNoBackgroundExport(true);
		}

		if (!pdfe->createPdf(this->filename, exportRange))
		{
			this->errorMsg = pdfe->getLastError();
		}

		delete pdfe;
	}
	else
	{
		exportPng();
	}
}

void CustomExportJob::afterRun()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	if (!this->lastError.empty())
	{
		Util::showErrorToUser(control->getGtkWindow(), this->lastError);
	}
}

