#include "ExportHelper.h"

namespace ExportHelper {

/**
 * @brief Export the input file as a bunch of image files (one per page)
 * @param input Path to the input file
 * @param output Path to the output file(s)
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param pngDpi Set dpi for Png files. Non positive values are ignored
 * @param pngWidth Set the width for Png files. Non positive values are ignored
 * @param pngHeight Set the height for Png files. Non positive values are ignored
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported image file has transparent background
 *
 *  The priority is: pngDpi overwrites pngWidth overwrites pngHeight
 *
 * @return 0 on success, -3 on export failure
 */
auto exportImg(Document* doc, const char* output, const char* range, int pngDpi, int pngWidth, int pngHeight,
               ExportBackgroundType exportBackground) -> int {

    fs::path const path(output);

    ExportGraphicsFormat format = EXPORT_GRAPHICS_PNG;

    if (path.extension() == ".svg") {
        format = EXPORT_GRAPHICS_SVG;
    }

    PageRangeVector exportRange;
    if (range) {
        exportRange = PageRange::parse(range, int(doc->getPageCount()));
    } else {
        exportRange.push_back(new PageRangeEntry(0, int(doc->getPageCount() - 1)));
    }

    DummyProgressListener progress;

    ImageExport imgExport(doc, path, format, exportBackground, exportRange);

    if (format == EXPORT_GRAPHICS_PNG) {
        if (pngDpi > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_DPI, pngDpi);
        } else if (pngWidth > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_WIDTH, pngWidth);
        } else if (pngHeight > 0) {
            imgExport.setQualityParameter(EXPORT_QUALITY_HEIGHT, pngHeight);
        }
    }

    imgExport.exportGraphics(&progress);

    for (PageRangeEntry* e: exportRange) { delete e; }
    exportRange.clear();

    std::string errorMsg = imgExport.getLastErrorMsg();
    if (!errorMsg.empty()) {
        g_message("Error exporting image: %s\n", errorMsg.c_str());
    }

    g_message("%s", _("Image file successfully created"));

    return 0;  // no error
}

/**
 * @brief Export the input file as pdf
 * @param input Path to the input file
 * @param output Path to the output file
 * @param range Page range to be parsed. If range=nullptr, exports the whole file
 * @param exportBackground If EXPORT_BACKGROUND_NONE, the exported pdf file has white background
 * @param progressiveMode If true, then for each xournalpp page, instead of rendering one PDF page, the page layers are
 * rendered one by one to produce as many pages as there are layers.
 *
 * @return 0 on success, -3 on export failure
 */
auto exportPdf(Document* doc, const char* output, const char* range, ExportBackgroundType exportBackground,
               bool progressiveMode) -> int {

    GFile* file = g_file_new_for_commandline_arg(output);

    std::unique_ptr<XojPdfExport> pdfe = XojPdfExportFactory::createExport(doc, nullptr);
    pdfe->setExportBackground(exportBackground);
    char* cpath = g_file_get_path(file);
    std::string path = cpath;
    g_free(cpath);
    g_object_unref(file);

    bool exportSuccess;  // Return of the export job

    if (range) {
        // Parse the range
        PageRangeVector exportRange = PageRange::parse(range, doc->getPageCount());
        // Do the export
        exportSuccess = pdfe->createPdf(path, exportRange, progressiveMode);
        // Clean up
        for (PageRangeEntry* e: exportRange) { delete e; }
        exportRange.clear();
    } else {
        exportSuccess = pdfe->createPdf(path, progressiveMode);
    }

    if (!exportSuccess) {
        g_error("%s", pdfe->getLastError().c_str());
    }

    g_message("%s", _("PDF file successfully created"));

    return 0;  // no error
}

}  // namespace ExportHelper
