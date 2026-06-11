#include "BookmarkDialog.h"

#include <glib/gi18n.h>

BookmarkDialog::BookmarkDialog(GtkWindow* parent, const std::string& initialName) {
    const char* title = initialName.empty() ? _("Add Bookmark") : _("Edit Bookmark");
    build(parent, title, initialName);
}

BookmarkDialog::~BookmarkDialog() {
    if (dialog) {
        gtk_widget_destroy(dialog);
        dialog = nullptr;
    }
}

void BookmarkDialog::build(GtkWindow* parent, const char* title, const std::string& initialName) {
    dialog = gtk_dialog_new_with_buttons(title, parent,
                                         static_cast<GtkDialogFlags>(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                         _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_ACCEPT, nullptr);

    okButton = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
    gtk_widget_set_can_default(okButton, TRUE);
    gtk_window_set_default(GTK_WINDOW(dialog), okButton);

    GtkWidget* content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_set_border_width(GTK_CONTAINER(content), 12);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_container_add(GTK_CONTAINER(content), box);

    GtkWidget* nameLabel = gtk_label_new_with_mnemonic(_("_Name:"));
    gtk_widget_set_halign(nameLabel, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(box), nameLabel, FALSE, FALSE, 0);

    nameEntry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(nameEntry), 40);
    gtk_entry_set_activates_default(GTK_ENTRY(nameEntry), TRUE);
    if (!initialName.empty()) {
        gtk_entry_set_text(GTK_ENTRY(nameEntry), initialName.c_str());
    }
    gtk_label_set_mnemonic_widget(GTK_LABEL(nameLabel), nameEntry);
    gtk_box_pack_start(GTK_BOX(box), nameEntry, TRUE, TRUE, 0);

    g_signal_connect(nameEntry, "changed", G_CALLBACK(onNameChanged), this);

    gtk_widget_show_all(content);
    updateOkSensitivity();
}

auto BookmarkDialog::run() -> bool {
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const gchar* raw = gtk_entry_get_text(GTK_ENTRY(nameEntry));
        resultName = raw ? std::string(raw) : "";
        return true;
    }
    return false;
}

auto BookmarkDialog::getName() const -> std::string { return resultName; }

void BookmarkDialog::onNameChanged(GtkEditable* editable, BookmarkDialog* self) {
    const gchar* text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (text) {
        glong len = g_utf8_strlen(text, -1);
        if (len > MAX_NAME_LENGTH) {
            const gchar* end = g_utf8_offset_to_pointer(text, MAX_NAME_LENGTH);
            gchar* truncated = g_strndup(text, static_cast<gsize>(end - text));

            g_signal_handlers_block_by_func(editable, (gpointer)onNameChanged, self);
            gtk_entry_set_text(GTK_ENTRY(editable), truncated);
            gtk_editable_set_position(editable, MAX_NAME_LENGTH);
            g_signal_handlers_unblock_by_func(editable, (gpointer)onNameChanged, self);

            g_free(truncated);
        }
    }
    self->updateOkSensitivity();
}

void BookmarkDialog::updateOkSensitivity() {
    const gchar* text = gtk_entry_get_text(GTK_ENTRY(nameEntry));
    glong len = text ? g_utf8_strlen(text, -1) : 0;
    gtk_widget_set_sensitive(okButton, len > 0 && len <= MAX_NAME_LENGTH);
}
