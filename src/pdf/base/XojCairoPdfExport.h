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

#include "control/jobs/BaseExportJob.h"
#include "control/jobs/ProgressListener.h"
#include "model/Document.h"

#include "XojPdfExport.h"
#include "filesystem.h"

class XojCairoPdfExport: public XojPdfExport {
public:
    XojCairoPdfExport(Document* doc, ProgressListener* progressListener);
    virtual ~XojCairoPdfExport();

public:
    virtual bool createPdf(fs::path const& file, bool progressiveMode);
    virtual bool createPdf(fs::path const& file, PageRangeVector& range, bool progressiveMode);
    virtual string getLastError();

    /**
     * Export without background
     */
    virtual void setExportBackground(ExportBackgroundType exportBackground);

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

private:
    Document* doc = nullptr;
    ProgressListener* progressListener = nullptr;

    cairo_surface_t* surface = nullptr;
    cairo_t* cr = nullptr;

    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    string lastError;
};
