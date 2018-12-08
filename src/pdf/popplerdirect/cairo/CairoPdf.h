/*
 * Xournal++
 *
 * Used to output cairo contents to PDF
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"
#include "pdf/popplerdirect/poppler/XojPopplerDocument.h"
#include "pdf/popplerdirect/poppler/XojPopplerPage.h"

#include <cairo.h>
#include <XournalType.h>

class CairoPdf
{
public:
	CairoPdf();
	virtual ~CairoPdf();

public:
	void drawPage(PageRef page);
	XojPdfPageSPtr getPage(int page);
	XojPopplerDocument& getDocument();

	void finalize();

private:
	static cairo_status_t writeOut(CairoPdf* pdf, unsigned char* data, unsigned int length);

private:
	XOJ_TYPE_ATTRIB;


	GString* data;

	XojPopplerDocument doc;

	cairo_surface_t* surface;
	cairo_t* cr;
};
