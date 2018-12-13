/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XojPdfExport.h"

#include "control/jobs/ProgressListener.h"
#include "model/Document.h"

class XojCairoPdfExport : public XojPdfExport
{
public:
	XojCairoPdfExport(Document* doc, ProgressListener* progressListener);
	virtual ~XojCairoPdfExport();

public:
	virtual bool createPdf(path file);
	virtual bool createPdf(path file, PageRangeVector& range);
	virtual string getLastError();

	/**
	 * Export without background
	 */
	virtual void setNoBackgroundExport(bool noBackgroundExport);

private:
	bool startPdf(path file);
	void endPdf();
	void exportPage(size_t page);

private:
	XOJ_TYPE_ATTRIB;

	Document* doc;
	ProgressListener* progressListener;

	cairo_surface_t* surface;
	cairo_t* cr;

	bool noBackgroundExport;

	string lastError;
};

