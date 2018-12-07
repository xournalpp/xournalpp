#include "XojPdfExportFactory.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "pdf/popplerdirect/PdfWriter.h"

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
	return new PdfExport(doc, listener);
}

void XojPdfExportFactory::setCompressPdfOutput(bool compress)
{
	PdfWriter::setCompressPdfOutput(compress);
}
