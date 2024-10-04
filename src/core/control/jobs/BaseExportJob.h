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

#include <functional>
#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWidget

#include "BlockingJob.h"  // for BlockingJob
#include "filesystem.h"   // for path

/**
 *  @brief List of types for the export of background components.
 *  The order must agree with the corresponding listBackgroundType in ui/exportSettings.ui.
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
    virtual void showFileChooser(std::function<void()> onFileSelected, std::function<void()> onCancel);

protected:
    virtual void addFilterToDialog(GtkFileChooser* dialog) = 0;
    static void addFileFilterToDialog(GtkFileChooser* dialog, const std::string& name, const std::string& mimetype);
    bool checkOverwriteBackgroundPDF(fs::path const& file) const;
    virtual bool testAndSetFilepath(const fs::path& file, const char* filterName = nullptr);

private:
protected:
    fs::path filepath;

    /**
     * Error message to show to the user
     */
    std::string errorMsg;

    class ExportType {
    public:
        std::string extension;
        std::string mimeType;

        ExportType(std::string ext, std::string mime): extension(std::move(ext)), mimeType(std::move(mime)) {}
    };
};
