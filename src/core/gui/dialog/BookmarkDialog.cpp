#include "BookmarkDialog.h"

#include <glib/gi18n.h>  // for _()

#include "gui/Builder.h"       // for Builder
#include "util/gtk4_helper.h"  // for gtk_editable_get_text, gtk_editable_set_text

BookmarkDialog::BookmarkDialog(GladeSearchpath* gladeSearchPath, const std::string& initialName,
                               std::function<void(std::string)> callback):
        callback(std::move(callback)) {
    Builder builder(gladeSearchPath, "bookmarkDialog.glade");
    window.reset(GTK_WINDOW(builder.get("bookmarkDialog")));
    nameEntry = builder.get("nameEntry");
    okButton = builder.get("okButton");

    const char* title = initialName.empty() ? _("Add Bookmark") : _("Edit Bookmark");
    gtk_window_set_title(window.get(), title);

    if (!initialName.empty()) {
        gtk_editable_set_text(GTK_EDITABLE(nameEntry), initialName.c_str());
    }

    updateOkSensitivity();

    g_signal_connect(nameEntry, "changed", G_CALLBACK(onNameChanged), this);

    g_signal_connect(builder.get("cancelButton"), "clicked", G_CALLBACK(+[](GtkButton*, gpointer self) {
                         gtk_window_close(static_cast<BookmarkDialog*>(self)->window.get());
                     }),
                     this);

    g_signal_connect(okButton, "clicked", G_CALLBACK(+[](GtkButton*, gpointer self) {
                         auto* dlg = static_cast<BookmarkDialog*>(self);
                         const char* text = gtk_editable_get_text(GTK_EDITABLE(dlg->nameEntry));
                         dlg->callback(text ? std::string(text) : "");
                         gtk_window_close(dlg->window.get());
                     }),
                     this);
}

void BookmarkDialog::onNameChanged(GtkEditable* editable, BookmarkDialog* self) {
    const char* text = gtk_editable_get_text(editable);
    if (text) {
        glong len = g_utf8_strlen(text, -1);
        if (len > MAX_NAME_LENGTH) {
            const char* end = g_utf8_offset_to_pointer(text, MAX_NAME_LENGTH);
            gchar* truncated = g_strndup(text, static_cast<gsize>(end - text));

            g_signal_handlers_block_by_func(editable, reinterpret_cast<gpointer>(onNameChanged), self);
            gtk_editable_set_text(editable, truncated);
            gtk_editable_set_position(editable, MAX_NAME_LENGTH);
            g_signal_handlers_unblock_by_func(editable, reinterpret_cast<gpointer>(onNameChanged), self);

            g_free(truncated);
        }
    }
    self->updateOkSensitivity();
}

void BookmarkDialog::updateOkSensitivity() {
    const char* text = gtk_editable_get_text(GTK_EDITABLE(nameEntry));
    glong len = text ? g_utf8_strlen(text, -1) : 0;
    gtk_widget_set_sensitive(okButton, len > 0 && len <= MAX_NAME_LENGTH);
}
