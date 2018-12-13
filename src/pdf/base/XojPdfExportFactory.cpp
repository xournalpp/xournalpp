#include "XojPdfExportFactory.h"

#include <config-features.h>

#ifdef ADVANCED_PDF_EXPORT_POPPLER
#include "pdf/popplerdirect/PdfExport.h"
#include "pdf/popplerdirect/PdfWriter.h"
#else
#include "XojCairoPdfExport.h"
#endif

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
#ifdef ADVANCED_PDF_EXPORT_POPPLER
	return new PdfExport(doc, listener);
#else
	return new XojCairoPdfExport(doc, listener);
#endif
}

void XojPdfExportFactory::setCompressPdfOutput(bool compress)
{
#ifdef ADVANCED_PDF_EXPORT_POPPLER
	PdfWriter::setCompressPdfOutput(compress);
#else
	// Currently not supported for Cairo export
#endif
}
