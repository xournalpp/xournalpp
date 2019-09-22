#include "XojPdfExportFactory.h"

#include <config-features.h>

#include "XojCairoPdfExport.h"

XojPdfExportFactory::XojPdfExportFactory()
{
}

XojPdfExportFactory::~XojPdfExportFactory()
{
}

XojPdfExport* XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener)
{
	return new XojCairoPdfExport(doc, listener);
}

