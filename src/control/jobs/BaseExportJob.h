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
    string getFilterName();

protected:
    void initDialog();
    virtual void addFilterToDialog() = 0;
    void addFileFilterToDialog(const string& name, const string& pattern);
    bool checkOverwriteBackgroundPDF(fs::path const& file) const;
    virtual bool isUriValid(string& uri);

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
        bool withoutBackground;

        ExportType(string ext, bool hideBg): extension(ext), withoutBackground(hideBg) {}
    };
};
