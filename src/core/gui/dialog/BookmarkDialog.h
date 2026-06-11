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

#include <string>

#include <gtk/gtk.h>

class BookmarkDialog {
public:
    static constexpr int MAX_NAME_LENGTH = 500;

    BookmarkDialog(GtkWindow* parent, const std::string& initialName = "");

    ~BookmarkDialog();

    auto run() -> bool;
    auto getName() const -> std::string;

private:
    void build(GtkWindow* parent, const char* title, const std::string& initialName);

    static void onNameChanged(GtkEditable* editable, BookmarkDialog* self);
    void updateOkSensitivity();

    GtkWidget* dialog = nullptr;
    GtkWidget* nameEntry = nullptr;
    GtkWidget* okButton = nullptr;

    std::string resultName;
};
