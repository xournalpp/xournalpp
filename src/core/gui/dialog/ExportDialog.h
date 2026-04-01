/*
 * Xournal++
 *
 * Dialog with export settings
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <functional>

#include <gtk/gtk.h>  // for GtkComboBox, GtkWindow

#include "control/jobs/BaseExportJob.h"  // for ExportBackgroundType
#include "control/jobs/ImageExport.h"    // for RasterImageQualityParameter
#include "gui/Builder.h"
#include "pdf/base/PdfExportBackend.h"
#include "util/ElementRange.h"  // for PageRangeVector
#include "util/raii/GtkWindowUPtr.h"

class GladeSearchpath;

namespace xoj::popup {
class ExportDialog {
public:
    ExportDialog(GladeSearchpath* gladeSearchPath, ExportGraphicsFormat format, size_t currentPage, size_t pageCount,
                 bool hasPdfBackground, std::function<void(const ExportDialog&)> callbackFun);
    ~ExportDialog();

public:
    bool isConfirmed() const;
    const PageRangeVector& getRange() const;
    bool progressiveModeSelected() const;
    ExportBackgroundType getBackgroundType() const;
    inline ExportBackend getPdfExportBackend() const { return pdfExportBackend; }

    /**
     * @brief Reads the quality parameter from the dialog
     *
     * @return The selected quality parameter
     */
    RasterImageQualityParameter getPngQualityParameter() const;

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    /**
     * @brief Handler for changes in combobox cbQuality
     */
    static void selectQualityCriterion(GtkComboBox* comboBox, ExportDialog* self);

    static void onSuccessCallback(ExportDialog* self);

private:
    xoj::util::GtkWindowUPtr window;

    size_t currentPage = 0;
    size_t pageCount = 0;

    bool confirmed = false;
    bool progressiveMode;
    ExportBackgroundType backgroundType;
    ExportBackend pdfExportBackend;
    RasterImageQualityParameter qualityParameter;
    PageRangeVector pageRanges;

    Builder builder;

    std::function<void(const ExportDialog&)> callbackFun;
};
};  // namespace xoj::popup
