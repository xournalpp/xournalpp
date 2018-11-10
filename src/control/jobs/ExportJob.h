/*
 * Xournal++
 *
 * An export job for the export dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "BlockingJob.h"
#include "ExportFormtType.h"

#include <PageRange.h>
#include <StringUtils.h>
#include <XournalType.h>

class ExportJob : public BlockingJob
{
public:
	ExportJob(Control* control, PageRangeVector selected, ExportFormtType type, int dpi, string filepath);

protected:
	virtual ~ExportJob();

public:
	virtual void run(bool noThreads);

private:
	bool createSurface(int id, double width, double height);
	bool freeSurface(int id);

	/**
	 * Get a filename with a numer, e.g. .../export-1.png, if the no is -1, return .../export.png
	 */
	string getFilenameWithNumber(int no);

private:
	XOJ_TYPE_ATTRIB;

	PageRangeVector selected;

	cairo_surface_t* surface;
	cairo_t* cr;

	int dpi;
	ExportFormtType type;

	/**
	 * Export path with filename
	 */
	string filepath;
};
