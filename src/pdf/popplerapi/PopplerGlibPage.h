/*
 * Xournal++
 *
 * PDF Page GLib Implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "pdf/base/XojPdfPage.h"

#include <poppler.h>


class PopplerGlibPage : public XojPdfPage
{
public:
	PopplerGlibPage(PopplerPage* page);
	PopplerGlibPage(const PopplerGlibPage& other);
	virtual ~PopplerGlibPage();
	void operator=(const PopplerGlibPage& other);

public:
	virtual double getWidth();
	virtual double getHeight();

	virtual void render(cairo_t* cr, bool forPrinting = false);

	virtual vector<XojPdfRectangle> findText(string& text);

	virtual int getPageId();

private:
	PopplerPage* page;
};

