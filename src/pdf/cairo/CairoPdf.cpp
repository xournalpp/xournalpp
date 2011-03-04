#include "CairoPdf.h"
#include "../../view/DocumentView.h"
#include <cairo/cairo-pdf.h>

CairoPdf::CairoPdf() {
	this->data = g_string_new("");

	this->surface = cairo_pdf_surface_create_for_stream((cairo_write_func_t) writeOut, this, 0, 0);
	this->cr = cairo_create(surface);
}

CairoPdf::~CairoPdf() {
	if (this->data) {
		g_string_free(this->data, true);
		this->data = NULL;
	}
}

cairo_status_t CairoPdf::writeOut(CairoPdf * pdf, unsigned char * data, unsigned int length) {
	g_string_append_len(pdf->data, (char *) data, length);
	return CAIRO_STATUS_SUCCESS;
}

void CairoPdf::drawPage(XojPage * page) {
	DocumentView view;

	cairo_pdf_surface_set_size(this->surface, page->getWidth(), page->getHeight());

	view.drawPage(page, this->cr);

	// next page
	cairo_show_page(this->cr);
}

void CairoPdf::finalize() {
	cairo_destroy(this->cr);
	cairo_surface_destroy(this->surface);

//	FILE * fp = fopen("/home/andreas/tmp/pdf/cairo.pdf", "w");
//	fwrite(this->data->str, 1, this->data->len, fp);
//	fclose(fp);

	doc.load(this->data->str, this->data->len);
	this->data->len = 0;
}

XojPopplerPage * CairoPdf::getPage(int page) {
	return doc.getPage(page);
}

XojPopplerDocument & CairoPdf::getDocument() {
	return doc;
}

