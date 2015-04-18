/*
 * Xournal++
 *
 * An export job for the export dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "BlockingJob.h"
#include "ExportFormtType.h"
#include <PageRange.h>
#include <StringUtils.h>
#include <XournalType.h>

#include <boost/filesystem/path.hpp>
using boost::filesystem::path;

class ExportJob : public BlockingJob
{
public:
	ExportJob(Control* control, PageRangeVector selected, ExportFormtType type,
			  int dpi, path filepath);

protected:
	virtual ~ExportJob();

public:
	virtual void run();

private:
	bool createSurface(int id, double width, double height);
	bool freeSurface(int id);

private:
	XOJ_TYPE_ATTRIB;


	PageRangeVector selected;

	cairo_surface_t* surface;
	cairo_t* cr;

	int dpi;
	ExportFormtType type;
	path filepath;
	string front, back;
};
