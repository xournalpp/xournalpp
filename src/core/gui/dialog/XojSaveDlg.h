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

#include <functional>
#include <optional>

#include <gtk/gtk.h>  // for GtkWindow

#include "util/raii/GtkWindowUPtr.h"

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
                                   std::function<void(std::optional<fs::path>)> callback);

    /**
     * Creates a save or export file dialog. The callback is called with std::nullopt if no path were selected
     * @param pathValidation May modify the given path. Returns true if the path is (now) valid for saving/exporting.
     * @param callback(path)
     */
    SaveExportDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle, const char* buttonLabel,
                     std::function<bool(fs::path&, const char* filterName)> pathValidation,
                     std::function<void(std::optional<fs::path>)> callback);
    ~SaveExportDialog() = default;

    inline GtkWindow* getWindow() const { return window.get(); }

private:
    /// Closes the dialog and calls the callback on `path`
    void close(std::optional<fs::path> path);

    xoj::util::GtkWindowUPtr window;
    std::function<void(std::optional<fs::path>)> callback;
    std::function<bool(fs::path&, const char* filterName)> pathValidation;
    gulong signalId{};
};
};  // namespace xoj
