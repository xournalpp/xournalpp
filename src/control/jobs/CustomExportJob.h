/*
 * Xournal++
 *
 * A customized export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BaseExportJob.h"

#include "view/DocumentView.h"

#include <PageRange.h>
#include <i18n.h>

const string EXPORT_PDF = _C("PDF files");
const string EXPORT_PDF_NOBG = _C("PDF with plain background");
const string EXPORT_PNG = _C("PNG graphics");
const string EXPORT_PNG_NOBG = _C("PNG with transparent background");
const string EXPORT_XOJ = _C("Xournal (Compatibility)");

class CustomExportJob : public BaseExportJob
{
public:
	CustomExportJob(Control* control);

protected:
	virtual ~CustomExportJob();

public:
	void run();

public:
	virtual bool showFilechooser();

protected:
	virtual void afterRun();

	virtual void addFilterToDialog();

	/**
	 * Export a single PNG page
	 */
	void exportPngPage(int pageId, int id, double zoom, DocumentView& view);

	/**
	 * Create one PNG file per page
	 */
	void exportPng();

	void createSurface(double width, double height);
	bool freeSurface(int id);
	string getFilenameWithNumber(int no);
	virtual bool isUriValid(string& uri);

private:
	XOJ_TYPE_ATTRIB;

	PageRangeVector exportRange;
	int pngDpi;

	cairo_surface_t* surface;
	cairo_t* cr;

	/**
	 * PDF Export, else PNG Export
	 */
	bool exportTypePdf;

	/**
	 * XOJ Export, else PNG Export
	 */
	bool exportTypeXoj;

	string lastError;

	string choosenFilterName;
};
