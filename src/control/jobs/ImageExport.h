/*
 * Xournal++
 *
 * Image export implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "view/DocumentView.h"

#include <PageRange.h>
#include <Path.h>
#include <XournalType.h>

#include <gtk/gtk.h>

class Document;
class ProgressListener;

enum ExportGraphicsFormat {
	EXPORT_GRAPHICS_UNDEFINED,
	EXPORT_GRAPHICS_PDF,
	EXPORT_GRAPHICS_PNG,
	EXPORT_GRAPHICS_SVG
};

class ImageExport
{
public:
	ImageExport(Document* doc, Path filename, ExportGraphicsFormat format, bool hideBackground, PageRangeVector& exportRange);
	virtual ~ImageExport();

public:
	/**
	 * PNG dpi
	 */
	void setPngDpi(int dpi);

	/**
	 * @return The last error message to show to the user
	 */
	string getLastErrorMsg();

	/**
	 * Create one Graphics file per page
	 */
	void exportGraphics(ProgressListener* stateListener);

private:
	/**
	 * Create surface
	 */
	void createSurface(double width, double height, int id);

	/**
	 * Free / store the surface
	 */
	bool freeSurface(int id);

	/**
	 * Get a filename with a number, e.g. .../export-1.png, if the no is -1, return .../export.png
	 */
	string getFilenameWithNumber(int no);

	/**
	 * Export a single Image page
	 */
	void exportImagePage(int pageId, int id, double zoom, ExportGraphicsFormat format, DocumentView& view);

public:
	/**
	 * Document to export
	 */
	Document* doc = nullptr;

	/**
	 * Filename for export
	 */
	Path filename;

	/**
	 * Export graphics format
	 */
	ExportGraphicsFormat format = EXPORT_GRAPHICS_UNDEFINED;

	/**
	 * Do not export the Background
	 */
	bool hideBackground = false;

	/**
	 * The range to export
	 */
	PageRangeVector& exportRange;

	/**
	 * PNG dpi
	 */
	int pngDpi = 300;

	/**
	 * Export surface
	 */
	cairo_surface_t* surface = nullptr;

	/**
	 * Cairo context
	 */
	cairo_t* cr = nullptr;

	/**
	 * The last error message to show to the user
	 */
	string lastError;
};
