/*
 * Xournal++
 *
 * Custom Poppler access library
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XOJ_POPPLERPAGE_H__
#define __XOJ_POPPLERPAGE_H__

#include <gtk/gtk.h>
#include <poppler/PDFDoc.h>
#include <poppler/Gfx.h>
#include <poppler/FontInfo.h>
#include <poppler/TextOutputDev.h>
#include <poppler/Catalog.h>
#include <poppler/OptionalContent.h>
#include <poppler/CairoOutputDev.h>

#include "../../util/MemoryCheck.h"

class Page;
class TextPage;
class Annots;
class XojPopplerImage;

class XojPopplerPage : public MemoryCheckObject {
private:
	XojPopplerPage(PDFDoc * doc, GMutex * docMutex, CairoOutputDev * outputDev, Page * page, int index);
	virtual ~XojPopplerPage();
public:
	double getWidth();
	double getHeight();

	void render(cairo_t * cr, bool forPrinting = false);

	GList * findText(const char * text);

	Page * getPage();

private:
	void initTextPage();

private:
	GMutex * renderMutex;
	GMutex * docMutex;

	CairoOutputDev * outputDev;
	PDFDoc * doc;
	Page * page;
	int index;

	TextPage * text;
	Annots * annots;

	friend class _IntPopplerDocument;
};

class XojPopplerRectangle {
public:
	XojPopplerRectangle();

public:
	double x1;
	double y1;
	double x2;
	double y2;
};

class XojPopplerImage {
public:
	XojPopplerImage();
	~XojPopplerImage();

public:

};

#endif /* __XOJ_POPPLERPAGE_H__ */
