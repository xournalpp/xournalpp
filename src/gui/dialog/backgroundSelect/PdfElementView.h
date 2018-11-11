/*
 * Xournal++
 *
 * PDF view
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseElementView.h"

#include <XournalType.h>

class XojPopplerPage;
class PdfPagesDialog;

class PdfElementView : public BaseElementView
{
public:
	PdfElementView(int id, XojPopplerPage* page, PdfPagesDialog* dlg);
	~PdfElementView();

protected:

	/**
	 * Paint the contents (without border / selection)
	 */
	virtual void paintContents(cairo_t* cr);

	/**
	 * Get the width in pixel, without shadow / border
	 */
	virtual int getContentWidth();

	/**
	 * Get the height in pixel, without shadow / border
	 */
	virtual int getContentHeight();

public:
	bool isUsed();
	void setUsed(bool used);
	void setHideUnused();

private:
	XOJ_TYPE_ATTRIB;

	XojPopplerPage* page;

	/**
	 * This page is already used as background
	 */
	bool used;
};
