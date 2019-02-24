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
#include <map>

enum ExportGraphicsFormat {
	EXPORT_GRAPHICS_UNDEFINED,
	EXPORT_GRAPHICS_PDF,
	EXPORT_GRAPHICS_PNG,
	EXPORT_GRAPHICS_SVG
};

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
	 * Export a single Image page
	 */
	void exportImagePage(int pageId, int id, double zoom, ExportGraphicsFormat format, DocumentView& view);

	/**
	 * Create one Graphics file per page
	 */
	void exportGraphics();

	void createSurface(double width, double height, int id);
	bool freeSurface(int id);
	string getFilenameWithNumber(int no);
	virtual bool isUriValid(string& uri);

private:
	XOJ_TYPE_ATTRIB;

	PageRangeVector exportRange;
	int pngDpi = 300;

	cairo_surface_t* surface = NULL;
	cairo_t* cr = NULL;

	/**
	 * Export graphics format
	 */
	ExportGraphicsFormat format = EXPORT_GRAPHICS_UNDEFINED;

	/**
	 * XOJ Export, else PNG Export
	 */
	bool exportTypeXoj = false;

	string lastError;

	string chosenFilterName;

	std::map<string, ExportType*> filters;
};
