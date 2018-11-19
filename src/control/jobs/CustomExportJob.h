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

class CustomExportJob : public BaseExportJob
{
public:
	CustomExportJob(Control* control);

protected:
	virtual ~CustomExportJob();

public:
	void run(bool noThreads);

public:
	virtual bool showFilechooser();

protected:
	virtual void addFilterToDialog();
	virtual void addExtensionToFilePath();

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

private:
	XOJ_TYPE_ATTRIB;

	GtkFileFilter* pdfFilter;

	PageRangeVector exportRange;
	int pngDpi;

	cairo_surface_t* surface;
	cairo_t* cr;

	/**
	 * PDF Export, else PNG Export
	 */
	bool exportTypePdf;
};
