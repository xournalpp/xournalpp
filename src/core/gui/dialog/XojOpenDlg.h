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

#include <gtk/gtk.h>  // for GtkWindow

#include "filesystem.h"  // for path

class Settings;

namespace xoj::OpenDlg {
void showOpenFileDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback);
/// @param callback(path, attachPdf)
void showAnnotatePdfDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path, bool)> callback);
void showOpenTemplateDialog(GtkWindow* parent, Settings* settings, std::function<void(fs::path)> callback);
};  // namespace xoj::OpenDlg
