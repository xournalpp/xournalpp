#include "ExportHandler.h"
#include "../gui/dialog/ExportDialog.h"
#include "../util/PageRange.h"

ExportHandler::ExportHandler() {
}

ExportHandler::~ExportHandler() {
}

void ExportHandler::runExportWithDialog(GladeSearchpath * gladeSearchPath, Document * doc, int current) {
	int count = doc->getPageCount();
	ExportDialog * dlg = new ExportDialog(gladeSearchPath, count, current);
	dlg->show();
	GList * selected = dlg->getRange();
	ExportFormtType type = dlg->getFormatType();
	int resolution = dlg->getResolution();
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
	if (type == EXPORT_FORMAT_PDF) {
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
