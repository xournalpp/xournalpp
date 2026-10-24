/*
 * Xournal++
 *
 * Helper functions for platform-native file choosers
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <optional>
#include <string>

#include <gtk/gtk.h>

#include "filesystem.h"

namespace xoj::NativeFileChooser {

using ConfigureCallback = std::function<void(GtkFileChooser*)>;
using FileSelectedCallback = std::function<void(std::optional<fs::path>, std::optional<std::string>)>;

bool isAvailable();

void show(GtkWindow* parent, const char* title, GtkFileChooserAction action, const char* acceptLabel,
          ConfigureCallback configure, FileSelectedCallback callback);

};  // namespace xoj::NativeFileChooser
