/*
 * Xournal++
 *
 * GTK Open dialog to select XOJ (or PDF) file with preview
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

namespace xoj::OpenDlg {
class OpenFileDialog {
public:
    OpenFileDialog(const char* title, std::function<void(fs::path, bool)> callback);
    OpenFileDialog(const char* title, std::function<void(fs::path)> callback);
    ~OpenFileDialog() = default;

    [[nodiscard]] inline GtkNativeDialog* getNativeDialog() const { return GTK_NATIVE_DIALOG(fileChooserNative.get()); }

    /// Closes the dialog and calls the callback on `path`
    void close(fs::path path, bool attach);

private:
    util::GObjectSPtr<GtkNativeDialog> fileChooserNative;

    std::function<void(fs::path, bool)> callback;
};

void showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback);
/// @param callback(path, attachPdf)
void showAnnotatePdfDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path, bool)> callback);
void showOpenTemplateDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback);

/// @param callback(path, attachImg)
void showOpenImageDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path, bool)> callback);

void showMultiFormatDialog(GtkWindow* parent, std::vector<std::string> formats, std::function<void(fs::path)> callback);
};  // namespace xoj::OpenDlg
