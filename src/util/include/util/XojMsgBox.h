/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <map>
#include <string>

#include <gtk/gtk.h>

#include "filesystem.h"

namespace XojMsgBox {

void setDefaultWindow(GtkWindow* win);
void showErrorToUser(GtkWindow* win, std::string msg);
void showPluginMessage(GtkWindow* win, const std::string& pluginName, const std::string& msg,
                       const std::map<int, std::string>& button, bool error,
                       std::function<void(GtkDialog*, gint)>&& on_response);
void replaceFileQuestion(GtkWindow* win, std::filesystem::path const& path,
                         std::function<void(GtkDialog*, gint)>&& on_response);
void showHelp(GtkWindow* win);
};  // namespace XojMsgBox
