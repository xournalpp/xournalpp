#include "XojPdfExportFactory.h"

#include <config-features.h>

#include "XojCairoPdfExport.h"

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener) -> std::unique_ptr<XojPdfExport> {
    return std::make_unique<XojCairoPdfExport>(doc, listener);
}
