/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <poppler/Catalog.h>
#include <poppler/FontInfo.h>
#include <poppler/Gfx.h>
#include <poppler/OptionalContent.h>
#include <poppler/PDFDoc.h>
#include <poppler/TextOutputDev.h>
#include "pdf/popplerdirect/workaround/workaround.h"

#include <StringUtils.h>
#include <XournalType.h>

#include <gtk/gtk.h>

class Page;
class TextPage;
class Annots;
class XojPopplerImage;

class XojPopplerPage
{
private:
	XojPopplerPage(PDFDoc* doc, GMutex* docMutex, CairoOutputDev* outputDev, Page* page, int index);
	virtual ~XojPopplerPage();
public:
	double getWidth();
	double getHeight();

	void render(cairo_t* cr, bool forPrinting = false);

	GList* findText(string& text);

	Page* getPage();

private:
	void initTextPage();

private:
	XOJ_TYPE_ATTRIB;

	GMutex renderMutex;
	GMutex* docMutex;

	CairoOutputDev* outputDev;
	PDFDoc* doc;
	Page* page;
	int index;

	TextPage* text;
	Annots* annots;

	friend class _IntPopplerDocument;
};

class XojPopplerRectangle
{
public:
	XojPopplerRectangle();

public:
	double x1;
	double y1;
	double x2;
	double y2;
};
