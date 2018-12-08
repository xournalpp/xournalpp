/*
 * Xournal++
 *
 * PDF Page Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <cairo/cairo.h>

#include <vector>
using std::vector;
#include <string>
using std::string;

#include <memory>

class XojPdfRectangle
{
public:
	XojPdfRectangle();

public:
	double x1;
	double y1;
	double x2;
	double y2;
};

class XojPdfPage
{
public:
	XojPdfPage();
	virtual ~XojPdfPage();

public:
	virtual double getWidth() = 0;
	virtual double getHeight() = 0;

	virtual void render(cairo_t* cr, bool forPrinting = false) = 0;

	virtual vector<XojPdfRectangle> findText(string& text) = 0;

private:
	XOJ_TYPE_ATTRIB;
};

typedef std::shared_ptr<XojPdfPage> XojPdfPageSPtr;

