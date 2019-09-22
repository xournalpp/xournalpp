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

#include <PageRange.h>
#include <Path.h>
#include <XournalType.h>

class XojPdfExport
{
public:
	XojPdfExport();
	virtual ~XojPdfExport();

public:
	virtual bool createPdf(Path file) = 0;
	virtual bool createPdf(Path file, PageRangeVector& range) = 0;
	virtual string getLastError() = 0;

	/**
	 * Export without background
	 */
	virtual void setNoBackgroundExport(bool noBackgroundExport);

private:
	};

