#include "CairoPdf.h"
#include "../../view/DocumentView.h"
#include <cairo/cairo-pdf.h>

CairoPdf::CairoPdf() {
	this->data = NULL;
}

CairoPdf::~CairoPdf() {
	if (this->data) {
		g_string_free(this->data, true);
		this->data = NULL;
	}
}

cairo_status_t CairoPdf::writeOut(CairoPdf *pdf, unsigned char *data, unsigned int length) {
	g_string_append_len(pdf->data, (char *) data, length);
	return CAIRO_STATUS_SUCCESS;
}

void CairoPdf::drawPage(XojPage * page) {
	DocumentView view;

	if (this->data == NULL) {
		this->data = g_string_new("");
	}

	cairo_surface_t * surface = cairo_pdf_surface_create_for_stream((cairo_write_func_t) writeOut, this, page->getWidth(), page->getHeight());
	cairo_t * cr = cairo_create(surface);

	view.drawPage(page, cr);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	// TODO: debug
	FILE * fp = fopen("/home/andreas/tmp/pdf/cairo.pdf", "w");
	fwrite(this->data->str, 1, this->data->len, fp);
	fclose(fp);

	doc.load(this->data->str, this->data->len);
	this->data->len = 0;
}

XojPopplerPage * CairoPdf::getPage() {
	return doc.getPage(0);
}

XojPopplerDocument & CairoPdf::getDocument() {
	return doc;
}

