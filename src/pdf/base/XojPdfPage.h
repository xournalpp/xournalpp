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

class XojPdfPage
{
public:
	XojPdfPage();
	virtual ~XojPdfPage();

public:
	virtual double getWidth() = 0;
	virtual double getHeight() = 0;

	virtual void render(cairo_t* cr, bool forPrinting = false) = 0;

private:
	XOJ_TYPE_ATTRIB;
};
