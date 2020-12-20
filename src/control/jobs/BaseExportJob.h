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

#include <string>
#include <vector>

#include "BlockingJob.h"
#include "PathUtil.h"
#include "XournalType.h"
#include "filesystem.h"

/**
 *  @brief List of types for the export of background components.
 *  Keep the order so that one can check for intermediate types using comparsion.
 */
enum ExportBackgroundType { EXPORT_BACKGROUND_NONE, EXPORT_BACKGROUND_ALL };

class Control;

class BaseExportJob: public BlockingJob {
public:
    BaseExportJob(Control* control, const string& name);

protected:
    virtual ~BaseExportJob();

public:
    virtual void afterRun();

public:
    virtual bool showFilechooser();
    string getFilterName() const;

protected:
    void initDialog();
    virtual void addFilterToDialog() = 0;
    void addFileFilterToDialog(const string& name, const string& pattern);
    bool checkOverwriteBackgroundPDF(fs::path const& file) const;
    virtual bool testAndSetFilepath(fs::path file);

private:
protected:
    GtkWidget* dialog = nullptr;

    fs::path filepath;

    /**
     * Error message to show to the user
     */
    string errorMsg;

    class ExportType {
    public:
        string extension;
        ExportBackgroundType exportBackground;

        ExportType(string ext, ExportBackgroundType exportBg): extension(ext), exportBackground(exportBg) {}
    };
};
