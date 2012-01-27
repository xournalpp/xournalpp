#include "PrintHandler.h"
#include "../model/Document.h"
#include "../control/settings/Settings.h"
#include "../view/DocumentView.h"

#include <math.h>

PrintHandler::PrintHandler() {
	XOJ_INIT_TYPE(PrintHandler);
}

PrintHandler::~PrintHandler() {
	XOJ_RELEASE_TYPE(PrintHandler);
}

void PrintHandler::drawPage(GtkPrintOperation * operation, GtkPrintContext * context, int pageNr, PrintHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, PrintHandler);

	cairo_t * cr = gtk_print_context_get_cairo_context(context);

	PageRef page = handler->doc->getPage(pageNr);
	if (!page.isValid()) {
		return;
	}

	double width = page.getWidth();
	double height = page.getHeight();

	if (width > height) {
		cairo_rotate(cr, M_PI_2);
		cairo_translate(cr, 0, -height);
	}

	XojPopplerPage * popplerPage = NULL;

	if (page.getBackgroundType() == BACKGROUND_TYPE_PDF) {
		int pgNo = page.getPdfPageNr();
		popplerPage = handler->doc->getPdfPage(pgNo);
		if (popplerPage) {
			popplerPage->render(cr, true);
		}
	}

	DocumentView view;
	view.drawPage(page, cr, true /* dont render eraseable */);
}

void PrintHandler::requestPageSetup(GtkPrintOperation * operation, GtkPrintContext * context, gint pageNr, GtkPageSetup *setup, PrintHandler * handler) {
	XOJ_CHECK_TYPE_OBJ(handler, PrintHandler);

	PageRef page = handler->doc->getPage(pageNr);
	if (!page.isValid()) {
		return;
	}

	double width = page.getWidth();
	double height = page.getHeight();

	if (width > height) {
		gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_LANDSCAPE);
	} else {
		gtk_page_setup_set_orientation(setup, GTK_PAGE_ORIENTATION_PORTRAIT);
	}

	GtkPaperSize * size = gtk_paper_size_new_custom("xoj-internal", "xoj-internal", width, height, GTK_UNIT_POINTS);
	gtk_page_setup_set_paper_size(setup, size);
	gtk_paper_size_free(size);
}

// TODO Another feature that's available in xournal but still missing in xournalpp is the "print paper ruling" checkbox. My workaround: Set the background to blank before printing/exporting.
void PrintHandler::print(Document * doc, int currentPage) {
	XOJ_CHECK_TYPE(PrintHandler);

	gchar * filename = g_build_filename(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, PRINT_CONFIG_FILE, NULL);

	GtkPrintSettings * settings = gtk_print_settings_new_from_file(filename, NULL);

	if (settings == NULL) {
		settings = gtk_print_settings_new();
	}

	this->doc = doc;

	GtkPrintOperation * op = gtk_print_operation_new();
	gtk_print_operation_set_print_settings(op, settings);
	gtk_print_operation_set_n_pages(op, doc->getPageCount());
	gtk_print_operation_set_current_page(op, currentPage);
	gtk_print_operation_set_job_name(op, "Xournal++");
	gtk_print_operation_set_unit(op, GTK_UNIT_POINTS);
	gtk_print_operation_set_use_full_page(op, true);
	g_signal_connect(op, "draw_page", G_CALLBACK(drawPage), this);
	g_signal_connect(op, "request-page-setup", G_CALLBACK(requestPageSetup), this);

	GtkPrintOperationResult res = gtk_print_operation_run(op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
	if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
		g_object_unref(settings);
		settings = gtk_print_operation_get_print_settings(op);
		gtk_print_settings_to_file(settings, filename, NULL);

		settings = NULL;
	}

	g_free(filename);
	g_object_unref(op);

	this->doc = NULL;
}

