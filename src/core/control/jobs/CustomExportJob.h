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

#include <map>     // for map
#include <string>  // for string

#include "util/ElementRange.h"  // for PageRangeVector

#include "BaseExportJob.h"  // for BaseExportJob, EXPORT_BACKGROUND_ALL
#include "ImageExport.h"    // for RasterImageQualityParameter, EXPORT_GRAP...
#include "filesystem.h"     // for path

class Control;


class CustomExportJob: public BaseExportJob {
public:
    CustomExportJob(Control* control);

protected:
    ~CustomExportJob() override;

public:
    void run() override;

public:
    bool showFilechooser() override;

protected:
    void afterRun() override;

    void addFilterToDialog() override;

    /**
     * Create one Graphics file per page
     */
    void exportGraphics();

    bool testAndSetFilepath(const fs::path& file) override;

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

    std::string lastError;

    std::string chosenFilterName;

    std::map<std::string, ExportType> filters;
};
