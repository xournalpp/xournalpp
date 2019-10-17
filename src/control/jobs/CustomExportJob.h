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
#include "ImageExport.h"

#include "view/DocumentView.h"

#include <PageRange.h>
#include <i18n.h>
#include <map>


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
	 * Create one Graphics file per page
	 */
	void exportGraphics();

	virtual bool isUriValid(string& uri);

private:
	/**
	 * The range to export
	 */
	PageRangeVector exportRange;

	/**
	 * PNG dpi
	 */
	int pngDpi = 300;

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
