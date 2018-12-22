#include "XojPdfExportFactory.h"

#include <config-features.h>

#include "XojCairoPdfExport.h"

XojPdfExportFactory::XojPdfExportFactory()
{
    XOJ_INIT_TYPE(XojPdfExportFactory);
}

XojPdfExportFactory::~XojPdfExportFactory()
{
    XOJ_RELEASE_TYPE(XojPdfExportFactory);
}

XojPdfExport* XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener)
{
	return new XojCairoPdfExport(doc, listener);
}

void XojPdfExportFactory::setCompressPdfOutput(bool compress)
{
	// Currently not supported for Cairo export
}
