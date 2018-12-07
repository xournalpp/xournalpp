/*
 * Xournal++
 *
 * Paints a pdf Page to a cairo context, handles special cases
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/PdfCache.h"
#include "pdf/base/XojPdfPage.h"

#include <cairo/cairo.h>

class PdfView
{
private:
	PdfView();
	virtual ~PdfView();

public:
	static void drawPage(PdfCache* cache, XojPdfPage* popplerPage, cairo_t* cr,
						 double zoom, double width, double height, bool forPrinting = false);
};
