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

#include "control/jobs/ImageExport.h"
#include "control/settings/Settings.h"
#include "gui/GladeGui.h"

#include "PageRange.h"

class ExportDialog: public GladeGui {
public:
    ExportDialog(GladeSearchpath* gladeSearchPath);
    virtual ~ExportDialog();

public:
    virtual void show(GtkWindow* parent);
    void initPages(int current, int count);
    bool isConfirmed() const;
    PageRangeVector getRange();
    bool progressiveMode();
    ExportBackgroundType getBackgroundType();

    /**
     * @brief Reads the quality parameter from the dialog
     *
     * @return The selected quality parameter
     */
    RasterImageQualityParameter getPngQualityParameter();

    /**
     * @brief Hides the quality settings
     */
    void removeQualitySetting();

    /**
     * @brief Show "progressive mode" checkbox and hide quality settings
     * (both cannot be shown at the same time)
     */
    void showProgressiveMode();

    /**
     * @brief Handler for changes in combobox cbQuality
     */
    static void selectQualityCriterion(GtkComboBox* comboBox, ExportDialog* self);

private:
    int currentPage = 0;
    int pageCount = 0;

    bool confirmed = false;
};
