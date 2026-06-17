/*
 * Xournal++
 *
 * Dialog for adding or editing a bookmark
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>
#include <string>

#include <gtk/gtk.h>

#include "util/raii/GtkWindowUPtr.h"  // for GtkWindowUPtr

class GladeSearchpath;

class BookmarkDialog {
public:
    static constexpr int MAX_NAME_LENGTH = 500;

    BookmarkDialog(GladeSearchpath* gladeSearchPath, const std::string& initialName,
                   std::function<void(std::string)> callback);

    GtkWindow* getWindow() const { return window.get(); }

private:
    static void onNameChanged(GtkEditable* editable, BookmarkDialog* self);
    void updateOkSensitivity();

    xoj::util::GtkWindowUPtr window;
    GtkWidget* nameEntry = nullptr;
    GtkWidget* okButton = nullptr;
    std::function<void(std::string)> callback;
};
