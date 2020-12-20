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

#include <string>
#include <vector>

#include "control/jobs/BaseExportJob.h"

#include "PageRange.h"
#include "XournalType.h"
#include "filesystem.h"

class XojPdfExport {
public:
    XojPdfExport();
    virtual ~XojPdfExport();

public:
    virtual bool createPdf(fs::path const& file, bool progressiveMode) = 0;
    virtual bool createPdf(fs::path const& file, PageRangeVector& range, bool progressiveMode) = 0;
    virtual string getLastError() = 0;

    /**
     * Export without background
     */
    virtual void setExportBackground(ExportBackgroundType exportBackground);

private:
};
