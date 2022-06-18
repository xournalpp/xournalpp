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

#include <gtk/gtk.h>  // for GtkWidget

#include "BlockingJob.h"  // for BlockingJob
#include "filesystem.h"   // for path

/**
 *  @brief List of types for the export of background components.
 *  The order must agree with the corresponding listBackgroundType in ui/exportSettings.glade.
 *  It is constructed so that one can check for intermediate types using comparison.
 */
enum ExportBackgroundType { EXPORT_BACKGROUND_NONE, EXPORT_BACKGROUND_UNRULED, EXPORT_BACKGROUND_ALL };

class Control;

class BaseExportJob: public BlockingJob {
public:
    BaseExportJob(Control* control, const std::string& name);

protected:
    ~BaseExportJob() override;

public:
    void afterRun() override;

public:
    virtual bool showFilechooser();
    std::string getFilterName() const;

protected:
    void initDialog();
    virtual void addFilterToDialog() = 0;
    void addFileFilterToDialog(const std::string& name, const std::string& pattern);
    bool checkOverwriteBackgroundPDF(fs::path const& file) const;
    virtual bool testAndSetFilepath(fs::path file);

private:
protected:
    GtkWidget* dialog = nullptr;

    fs::path filepath;

    /**
     * Error message to show to the user
     */
    std::string errorMsg;

    class ExportType {
    public:
        std::string extension;

        ExportType(std::string ext): extension(ext) {}
    };
};
