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

#include <memory>  // for unique_ptr
#include <string>  // for string

#include "BaseExportJob.h"  // for BaseExportJob
#include "filesystem.h"     // for path

class Control;
struct ExportParameters;

class CustomExportJob: public BaseExportJob {
public:
    CustomExportJob(Control* control, fs::path output, std::unique_ptr<ExportParameters> params);

protected:
    ~CustomExportJob() override;

public:
    void run() override;

protected:
    void afterRun() override;

    /**
     * Create one Graphics file per page
     */
    void exportGraphics();


private:
    std::unique_ptr<ExportParameters> parameters;

    std::string lastError;
};
