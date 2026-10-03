/*
 * Xournal++
 *
 * Base class for Exports
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include "BlockingJob.h"  // for BlockingJob
#include "filesystem.h"   // for path

class Control;

class BaseExportJob: public BlockingJob {
public:
    BaseExportJob(Control* control, const std::string& name);

protected:
    ~BaseExportJob() override;

public:
    void afterRun() override;

protected:
    bool checkOverwriteBackgroundPDF(fs::path const& file) const;

    /**
     * Test if the given path is valid
     *
     * Returns true if the path is validated.
     */
    bool testFilepath(const fs::path& file) const;

protected:
    fs::path filepath;

    /**
     * Error message to show to the user
     */
    std::string errorMsg;
};
