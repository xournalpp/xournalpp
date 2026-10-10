/*
 * Xournal++
 *
 * GTK Save/Export dialog to select destination file
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>

#include <gtk/gtk.h>  // for GtkWindow

#include "util/move_only_function.h"
#include "util/raii/GtkWindowUPtr.h"

#include "FileChooserFiltersHelper.h"
#include "filesystem.h"  // for path

class Settings;

namespace xoj {
/// Helper class, for a single dialog
class SaveExportDialog {
public:
    /**
     * Shows a save file dialog. The callback is called with std::nullopt if no path were selected
     */
    static void showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                   xoj::util::move_only_function<void(std::optional<fs::path>)> callback);
    /**
     * Shows an save file dialog for exporting. The callback is called with std::nullopt if no path were selected
     */
    static void showExportFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                                     const FileType& filetype,
                                     xoj::util::move_only_function<void(std::optional<fs::path>)> callback);

    /**
     * Creates a save or export file dialog. The callback is called with std::nullopt if no path were selected
     * @param callback(path)
     */
    SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle, const char* buttonLabel,
                     const FileType& filetype, xoj::util::move_only_function<void(std::optional<fs::path>)> callback);
    SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle, const char* buttonLabel,
                     xoj::util::move_only_function<void(std::optional<fs::path>)> callback);
    ~SaveExportDialog() = default;

    inline GtkNativeDialog* getNativeDialog() const { return window.get(); }
    inline GtkFileChooser* getFileChooser() const { return GTK_FILE_CHOOSER(window.get()); }

private:
    /// Closes the dialog and calls the callback on `path`
    void close(std::optional<fs::path> path);

    xoj::util::GtkNativeDialogUPtr window;
    xoj::util::move_only_function<void(std::optional<fs::path>)> callback;
    gulong signalId{};
};
};  // namespace xoj
