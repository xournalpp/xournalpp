#include "XojPdfExportFactory.h"

#include <config-features.h>

#include "XojCairoPdfExport.h"

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(Document* doc, ProgressListener* listener) -> XojPdfExport* {
    return new XojCairoPdfExport(doc, listener);
}
