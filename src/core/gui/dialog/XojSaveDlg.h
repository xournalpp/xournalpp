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

#include "filesystem.h"  // for path

class Settings;

namespace xoj::SaveDlg {
/**
 * Shows a save file dialog. The callback is called with std::nullopt if no path were selected
 */
void showSaveFileDialog(GtkWindow* parent, Settings* settings, fs::path suggestedPath,
                        std::function<void(std::optional<fs::path>)> callback);
};  // namespace xoj::SaveDlg
