#include "ExportHandler.h"
#include "../gui/dialog/ExportDialog.h"
#include "../util/PageRange.h"
#include <cairo-ps.h>
#include <cairo-svg.h>

ExportHandler::ExportHandler() {
	this->surface = NULL;
	this->cr = NULL;

	this->type = EXPORT_FORMAT_PNG;
	this->dpi = 1;
}

ExportHandler::~ExportHandler() {
}

bool ExportHandler::createSurface(int id, double width, double height) {
	if (this->type == EXPORT_FORMAT_EPS) {
		char * path = NULL;
		if (id == -1) {
			path = g_strdup_printf("%s%c%s.eps", this->folder.c_str(), G_DIR_SEPARATOR, this->filename.c_str());
		} else {
			path = g_strdup_printf("%s%c%s%i.eps", this->folder.c_str(), G_DIR_SEPARATOR, this->filename.c_str(), id);
		}

		this->surface = cairo_ps_surface_create(path, width, height);
		g_free(path);

		cairo_ps_surface_set_eps(this->surface, true);

		this->cr = cairo_create(this->surface);
	} else if (this->type == EXPORT_FORMAT_PNG) {
		this->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width * this->dpi / 72.0, height * this->dpi / 72.0);
		this->cr = cairo_create(this->surface);
		double factor = this->dpi / 72.0;
		cairo_scale(this->cr, factor, factor);
	} else if (this->type == EXPORT_FORMAT_SVG) {
		char * path = NULL;
		if (id == -1) {
			path = g_strdup_printf("%s%c%s.svg", this->folder.c_str(), G_DIR_SEPARATOR, this->filename.c_str());
		} else {
			path = g_strdup_printf("%s%c%s%i.svg", this->folder.c_str(), G_DIR_SEPARATOR, this->filename.c_str(), id);
		}

		this->surface = cairo_svg_surface_create(path, width, height);
		g_free(path);

		this->cr = cairo_create(this->surface);
	} else {
		g_error("ExportHandler::createSurface unknown ExportFormtType %i", this->type);
		return false;
	}

	return true;
}

void ExportHandler::freeSurface(int id) {

}

void ExportHandler::runExportWithDialog(GladeSearchpath * gladeSearchPath, Settings * settings, Document * doc, int current) {
	int count = doc->getPageCount();
	ExportDialog * dlg = new ExportDialog(gladeSearchPath, settings, count, current);
	dlg->show();

	GList * selected = dlg->getRange();
	this->type = dlg->getFormatType();
	this->dpi = dlg->getPngDpi();
	this->folder = dlg->getFolder();
	this->filename = dlg->getFilename();

	delete dlg;

	if (selected == NULL) {
		return;
	}

	int onePage = -1;
	if (selected->next == NULL) {
		PageRangeEntry * e = (PageRangeEntry *) selected->data;
		if (e->first == e->last) {
			onePage = e->first;
		}
	}

	// pdf, supports multiple Pages per document
	if (this->type == EXPORT_FORMAT_PDF) {
		// TODO: !!!!!!!!!!!!!!!!!PDF EXPORT
	} else { // all other formats need one file per page
		if (onePage != -1) { // only one page to export

		} else { // multiple pages to export
			char selectedPages[count];
			for (int i = 0; i < count; i++) {
				selectedPages[i] = 0;
			}
			for (GList * l = selected; l != NULL; l = l->next) {
				PageRangeEntry * e = (PageRangeEntry *) l->data;
				for (int x = e->first; x <= e->last; x++) {
					selectedPages[x] = 1;
				}
			}

			for (int i = 0; i < count; i++) {
				printf("%i: %i\n", i, selectedPages[i]);
			}
		}
	}

	for (GList * l = selected; l != NULL; l = l->next) {
		PageRangeEntry * e = (PageRangeEntry *) l->data;
		delete e;
	}
	g_list_free(selected);
	selected = NULL;
}
