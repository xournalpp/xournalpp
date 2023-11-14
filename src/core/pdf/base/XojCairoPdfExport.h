/*
 * Xournal++
 *
 * PDF Document Export Abstraction Interface
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <string>   // for string

#include <cairo.h>    // for CAIRO_VERSION, CAIRO_VERSION...
#include <gtk/gtk.h>  // for GtkTreeModel

#include "control/jobs/BaseExportJob.h"  // for ExportBackgroundType, EXPORT...
#include "util/ElementRange.h"           // for PageRangeVector

#include "XojPdfExport.h"  // for XojPdfExport
#include "filesystem.h"    // for path

class Document;
class ProgressListener;

class XojCairoPdfExport: public XojPdfExport {
public:
    XojCairoPdfExport(const Document* doc, ProgressListener* progressListener);
    ~XojCairoPdfExport() override;

public:
    bool createPdf(fs::path const& file, bool progressiveMode) override;
    bool createPdf(fs::path const& file, const PageRangeVector& range, bool progressiveMode) override;
    std::string getLastError() override;

    /**
     * Export without background
     */
    void setExportBackground(ExportBackgroundType exportBackground) override;

private:
    bool startPdf(const fs::path& file);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 16, 0)
    /**
     * Populate the outline of the generated PDF using the outline of the
     * background PDF.
     *
     * This requires features available only in cairo 1.16 or newer.
     *
     * @param tocModel The Document's content model. Does nothing if set to null.
     */
    void populatePdfOutline(GtkTreeModel* tocModel);
#endif
    void endPdf();
    void exportPage(size_t page);
    /**
     * Export as a PDF document where each additional layer creates a
     * new page */
    void exportPageLayers(size_t page);

    /**
     * @brief Select layers to export by parsing str
     * @param rangeStr A string parsed to get a list of layers
     */
    void setLayerRange(const char* rangeStr) override;

private:
    const Document* doc = nullptr;
    ProgressListener* progressListener = nullptr;

    cairo_surface_t* surface = nullptr;
    cairo_t* cr = nullptr;

    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    std::string lastError;

    std::unique_ptr<LayerRangeVector> layerRange;
};
