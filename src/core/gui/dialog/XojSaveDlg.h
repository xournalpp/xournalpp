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

#include "util/raii/GObjectSPtr.h"

#include "filesystem.h"  // for path

class Settings;

namespace xoj::SaveDlg {
class SaveFileDialog {
public:
    SaveFileDialog(Settings* settings, fs::path suggestedPath, const char* windowTitle, const char* buttonLabel,
                   std::function<bool(fs::path&, const char* filterName)> pathValidation,
                   std::function<void(std::optional<fs::path>)> callback);
    ~SaveFileDialog() = default;

    [[nodiscard]] inline GtkNativeDialog* getNativeDialog() const { return GTK_NATIVE_DIALOG(fileChooserNative.get()); }

    /// Closes the dialog and calls the callback on `path`
    void close(std::optional<fs::path> path);

private:
    util::GObjectSPtr<GtkNativeDialog> fileChooserNative;
    std::function<void(std::optional<fs::path>)> callback;
    std::function<bool(fs::path&, const char* filterName)> pathValidation;
    gulong signalId{};
};

/**
 * Shows a save file dialog. The callback is called with std::nullopt if no path were selected
 */
void showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                        std::function<void(std::optional<fs::path>)> callback);

};  // namespace xoj::SaveDlg
