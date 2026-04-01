#include "XojPdfExportFactory.h"

#include "model/Document.h"

#include "QPdfExport.h"
#include "XojCairoPdfExport.h"  // for XojCairoPdfExport

class XojPdfExport;

XojPdfExportFactory::XojPdfExportFactory() = default;

XojPdfExportFactory::~XojPdfExportFactory() = default;

auto XojPdfExportFactory::createExport(const Document* doc, ProgressListener* listener, ExportBackend backend)
        -> std::unique_ptr<XojPdfExport> {
    if (!doc->getPdfFilepath().empty()) {
        switch (backend) {
            case ExportBackend::DEFAULT:  // fallback to qpdf/podofo/mupdf/cairo in that order
#ifdef ENABLE_QPDF
            case ExportBackend::QPDF:
                return std::make_unique<QPdfExport>(doc, listener);
#endif
            case ExportBackend::CAIRO:
            default:  // The requested backend has not been included in this build
                return std::make_unique<XojCairoPdfExport>(doc, listener);
        }
    }
    return std::make_unique<XojCairoPdfExport>(doc, listener);
}
