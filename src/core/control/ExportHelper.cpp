#include "ExportHelper.h"

#include <algorithm>  // for max
#include <memory>     // for unique_ptr, allocator
#include <stdexcept>  // for invalid_argument, runtime_error
#include <string>     // for string

#include <gio/gio.h>  // for g_file_new_for_commandlin...
#include <glib.h>     // for g_message, g_error

#include "control/jobs/ImageExport.h"       // for ImageExport, EXPORT_GRAPH...
#include "control/jobs/ProgressListener.h"  // for DummyProgressListener
#include "model/Document.h"                 // for Document
#include "pdf/base/XojPdfExport.h"          // for XojPdfExport
#include "pdf/base/XojPdfExportFactory.h"   // for XojPdfExportFactory
#include "util/ElementRange.h"              // for parse, PageRangeVector
#include "util/i18n.h"                      // for _
#include "util/raii/GObjectSPtr.h"          // for GObjectSPtr

#include "filesystem.h"  // for operator==, path

namespace ExportHelper {

void exportImg(Document* doc, fs::path outfile, const char* range, const char* layerRange, int pngDpi, int pngWidth,
               int pngHeight, ExportBackgroundType exportBackground) {

    ExportGraphicsFormat format = EXPORT_GRAPHICS_PNG;

    if (outfile.extension() == ".svg") {
        format = EXPORT_GRAPHICS_SVG;
    }

    PageRangeVector exportRange;
    if (range) {
        exportRange = ElementRange::parse(range, doc->getPageCount());
    } else {
        exportRange.emplace_back(0, doc->getPageCount() - 1);
    }

    DummyProgressListener progress;

    ImageExport imgExport(doc, std::move(outfile), format, exportBackground, exportRange);

    if (format == EXPORT_GRAPHICS_PNG) {
        if (pngDpi > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_DPI, pngDpi);
        } else if (pngWidth > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_WIDTH, pngWidth);
        } else if (pngHeight > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_HEIGHT, pngHeight);
        }
    }

    imgExport.setLayerRange(layerRange);

    imgExport.exportGraphics(&progress);

    std::string errorMsg = imgExport.getLastErrorMsg();
    if (!errorMsg.empty()) {
        throw std::runtime_error(errorMsg);
    }

    g_message("%s", _("Image file successfully created"));
}

void exportPdf(Document* doc, const fs::path& output, const char* range, const char* layerRange,
               ExportBackgroundType exportBackground, bool progressiveMode, ExportBackend backend) {
    std::unique_ptr<XojPdfExport> pdfe = XojPdfExportFactory::createExport(doc, nullptr, backend);
    pdfe->setExportBackground(exportBackground);

    // Check if we're trying to overwrite the background PDF file
    auto backgroundPDF = doc->getPdfFilepath();
    try {
        if (!backgroundPDF.empty() && fs::exists(backgroundPDF)) {
            if (fs::weakly_canonical(output) == fs::weakly_canonical(backgroundPDF)) {
                throw std::invalid_argument{_("Do not overwrite the background PDF! This will cause errors!")};
            }
        }
    } catch (const fs::filesystem_error& fe) {
        throw std::runtime_error{FS(_F("The check for overwriting the background failed with: {1}") % fe.what())};
    }

    bool exportSuccess = 0;  // Return of the export job

    pdfe->setLayerRange(layerRange);

    if (range) {
        // Parse the range
        PageRangeVector exportRange = ElementRange::parse(range, doc->getPageCount());
        // Do the export
        exportSuccess = pdfe->createPdf(output, exportRange, progressiveMode);
    } else {
        exportSuccess = pdfe->createPdf(output, progressiveMode);
    }

    if (!exportSuccess) {
        throw std::runtime_error{pdfe->getLastError()};
    }

    g_message("%s", _("PDF file successfully created"));
}

}  // namespace ExportHelper
