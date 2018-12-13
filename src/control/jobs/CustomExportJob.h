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

class Document;
class PageType;

const string EXPORT_PDF = "PDF files";
const string EXPORT_PDF_NOBG = "PDF with plain background";
const string EXPORT_PNG = "PNG graphics";
const string EXPORT_PNG_NOBG = "PNG with transparent background";
const string EXPORT_XOJ = "Xournal (Compatibility)";

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
