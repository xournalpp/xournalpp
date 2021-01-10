/*
 * Xournal++
 *
 * A customized export
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <map>

#include "view/DocumentView.h"

#include "BaseExportJob.h"
#include "ImageExport.h"
#include "PageRange.h"
#include "i18n.h"


class CustomExportJob: public BaseExportJob {
public:
    CustomExportJob(Control* control);

protected:
    virtual ~CustomExportJob();

public:
    void run();

public:
    virtual bool showFilechooser();

protected:
    virtual void afterRun();

    virtual void addFilterToDialog();

    /**
     * Create one Graphics file per page
     */
    void exportGraphics();

    bool testAndSetFilepath(fs::path file) override;

private:
    /**
     * The range to export
     */
    PageRangeVector exportRange;

    /**
     * @brief Quality parameter for PNG exports
     */
    RasterImageQualityParameter pngQualityParameter = RasterImageQualityParameter();

    /**
     * Export graphics format
     */
    ExportGraphicsFormat format = EXPORT_GRAPHICS_UNDEFINED;

    /**
     * XOJ Export, else PNG Export
     */
    bool exportTypeXoj = false;

    /**
     * Background export type
     */
    ExportBackgroundType exportBackground = EXPORT_BACKGROUND_ALL;

    /**
     * Export all Layers progressively
     */
    bool progressiveMode = false;

    string lastError;

    string chosenFilterName;

    std::map<string, ExportType*> filters;
};
