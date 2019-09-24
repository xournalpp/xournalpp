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
	virtual bool createPdf(Path file);
	virtual bool createPdf(Path file, PageRangeVector& range);
	virtual string getLastError();

	/**
	 * Export without background
	 */
	virtual void setNoBackgroundExport(bool noBackgroundExport);

private:
	bool startPdf(Path file);
	void endPdf();
	void exportPage(size_t page);

private:
	Document* doc = nullptr;
	ProgressListener* progressListener = nullptr;

	cairo_surface_t* surface = nullptr;
	cairo_t* cr = nullptr;

	bool noBackgroundExport = false;

	string lastError;
};

