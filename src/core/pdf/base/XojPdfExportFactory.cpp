#include "XojPdfExportFactory.h"

#include "XojCairoPdfExport.h"  // for XojCairoPdfExport

class XojPdfExport;

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener) -> std::unique_ptr<XojPdfExport> {
    return std::make_unique<XojCairoPdfExport>(doc, listener);
}
