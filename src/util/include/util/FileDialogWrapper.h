/*
 * Xournal++
 *
 * PopupWindow base class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/gtk4_helper.h"

namespace xoj::popup {

/**
 * @brief The class FileDialogWrapper allows a safe non-blocking creation and display of a file dialogs.
 * It shows the file dialog (upon a call to show()) and tasks a callback function to actually delete the popup once it
 * has been closed by the user.
 */
template <class FileDialogType>
class FileDialogWrapper {
public:
    FileDialogWrapper() = delete;
    FileDialogWrapper(const FileDialogWrapper&) = delete;
    FileDialogWrapper(FileDialogWrapper&&) = delete;

    template <class... Args>
    explicit FileDialogWrapper(Args&&... args) {
        dialog = new FileDialogType(std::forward<Args>(args)...);
    }

    // dialog must destory itself on close
    ~FileDialogWrapper() = default;

    void show(GtkWindow* parent) const {
        gtk_native_dialog_set_transient_for(dialog->getNativeDialog(), parent);
        gtk_native_dialog_set_modal(dialog->getNativeDialog(), true);

        gtk_native_dialog_show(dialog->getNativeDialog());
    }

    FileDialogType* getPopup() const {
        xoj_assert_message(dialog, "Do not call getPopup() after show()!");
        return dialog;
    }

private:
    FileDialogType* dialog = nullptr;
};
};  // namespace xoj::popup
