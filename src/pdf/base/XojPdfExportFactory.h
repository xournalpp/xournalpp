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

#include <XournalType.h>

class Document;
class ProgressListener;

class XojPdfExportFactory
{
private:
	XojPdfExportFactory();
	virtual ~XojPdfExportFactory();

public:
	static XojPdfExport* createExport(Document* doc, ProgressListener* listener);

private:
	};

